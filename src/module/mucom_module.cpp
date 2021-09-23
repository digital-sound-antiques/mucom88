// MucomModule 
// BouKiCHi 2019

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#define _CHDIR chdir
#else
#include <direct.h>
#define _CHDIR _chdir
#endif

// 大小区別なし
#ifdef __APPLE__
#define STRCASECMP strcasecmp
#else
#ifdef _WIN32
#define STRCASECMP _strcmpi
#else
#define STRCASECMP strcasecmp
#endif
#endif

#include "mucom_module.h"

#define MUCOM_DEFAULT_PCMFILE "mucompcm.bin"
#define DEFAULT_OUTFILE "mucom88.mub"

MucomModule::MucomModule() {
	audioRate = 44100;
	pcmfile = MUCOM_DEFAULT_PCMFILE;
	outfile = DEFAULT_OUTFILE;
	voicefile = NULL;
	resultText = NULL;
	volume = 1.0f;
	fade = nullptr;

	tag = new MucomTag();

	enableFade = false;
	playedFrames = 0;
	defaultLength = 180;
	seekPosition = -1;
}

MucomModule::~MucomModule() {
	Close();
}

void MucomModule::SetWorkDir(const char* workingDirectory) {
	_CHDIR(workingDirectory);
}

void MucomModule::SetRate(int rate) {
	audioRate = rate;
}

void MucomModule::SetPCM(const char* file) {
	pcmfile = file;
}

void MucomModule::SetVoice(const char* file) {
	voicefile = file;
}

void MucomModule::SetVolume(double vol) {
	volume = vol;
}

int MucomModule::GetRate() {
	return audioRate;
}

void MucomModule::UseFader(bool enable) {
	enableFade = enable;
}

void MucomModule::SetDefaultLength(int length) {
	defaultLength = length;
}

void MucomModule::Seek(int pos)
{
	seekPosition = pos;
}

int MucomModule::GetPosition() {
	return playedFrames / audioRate;
}

bool MucomModule::IsEnd()
{
	int pos = GetPosition();
	return length <= pos;
}

/// <summary>
/// 曲の秒数を得る
/// </summary>
int MucomModule::GetLength() {
	return length;
}

/// <summary>
/// 曲の秒数を得る
/// </summary>
int MucomModule::GetLengthFromTag() {
	std::string time = tag->time;
	if (time.length() > 0) {
		return std::atoi(time.c_str());
	}
	return defaultLength;
}

bool MucomModule::Open(const char* workingDirectory, const char* songFilename) {
	SetWorkDir(workingDirectory);
	return Open(songFilename);
}

/// <summary>
/// 曲タグの取得
/// </summary>
void MucomModule::LoadSongTag(const char* songFilename) {

	char fname[_MAX_PATH];
	strcpy(fname, songFilename);

	// ドライバモードの取得
	if (cmpopt & MUCOM_CMPOPT_COMPILE) {
		driverMode = mucom->GetDriverMode(fname);
	}
	else {
		driverMode = mucom->GetDriverModeMUB(fname);
	}

	// タグの取得
	LoadTag();
}

bool MucomModule::Open(const char* songFilename)
{
	mucom = new CMucom();
	cmpopt = MUCOM_CMPOPT_COMPILE;
	mucom->Init(NULL, cmpopt, audioRate);

	const char *ext = strrchr(songFilename, '.');
	bool is_mub = (ext != NULL && STRCASECMP(ext, ".mub") == 0);

	// 入力ファイルがMMLの場合
	if (!is_mub) {
		LoadSongTag(songFilename);
		mucom->SetDriverMode(driverMode);

		mucom->Reset(cmpopt);
		if (pcmfile) mucom->LoadPCM(pcmfile);
		if (voicefile) mucom->LoadFMVoice(voicefile);
		int cr = mucom->CompileFile(songFilename, outfile);

		AddResultBuffer(GetMucomMessage());

		FreeMucom();

		if (cr != 0) return false;
	}

	// 再生用に再度準備
	mucom = new CMucom();

	cmpopt = MUCOM_CMPOPT_STEP;
	mucom->Init(NULL, cmpopt, audioRate);

	// 入力ファイルがMUBの場合
	if (is_mub) {
		LoadSongTag(songFilename);
	}

	mucom->SetDriverMode(driverMode);

	mucom->Reset(0);
	if (mucom->LoadMusic(outfile) < 0) {
		AddResultBuffer(GetMucomMessage());
		return false;
	}
	return true;
}


/// <summary>
/// 曲データ(.mub/.muc)をバイナリで開く
/// .mucの場合は末端を\0でターミネートすること
/// </summary>
bool MucomModule::OpenMemory(uint8_t *data, int size, const char *path)
{
	mucom = new CMucom();

	const char* ext = strrchr(path, '.');
	bool is_mub = (ext != NULL && STRCASECMP(ext, ".mub") == 0);

	// 入力ファイルがMMLの場合
	if (!is_mub) {
		cmpopt = MUCOM_CMPOPT_COMPILE | MUCOM_CMPOPT_STEP;
		mucom->Init(NULL, cmpopt, audioRate);

		driverMode = mucom->GetDriverModeMem((char *)data);

		// タグの取得
		LoadTag();

		mucom->SetDriverMode(driverMode);

		mucom->Reset(cmpopt);
		if (pcmfile) mucom->LoadPCM(pcmfile);
		if (voicefile) mucom->LoadFMVoice(voicefile);
		int cr = mucom->CompileMem((char *)data);

		AddResultBuffer(GetMucomMessage());

		if (cr != 0) return false;
	}

	// 入力ファイルがMUBの場合
	if (is_mub) {
		cmpopt = MUCOM_CMPOPT_STEP;
		mucom->Init(NULL, cmpopt, audioRate);

		driverMode = mucom->GetDriverModeMemMUB(data, size);
		LoadTag();

		mucom->SetDriverMode(driverMode);
	}


	mucom->Reset(0);

	return true;
}

void MucomModule::LoadTag()
{
	tag->LoadTag(mucom);
	length = GetLengthFromTag();
}


const char* MucomModule::GetMucomMessage() {
	mucom->PrintInfoBuffer();
	return mucom->GetMessageBuffer();
}

void MucomModule::AddResultBuffer(const char* text) {
	int len = strlen(text);
	if (resultText != NULL) len += strlen(resultText);
	char* ptr = new char[len + 1];
	if (resultText != NULL) strcpy(ptr, resultText); else ptr[0] = 0x00;
	strcat(ptr, text);
	FreeResultBuffer();
	resultText = ptr;
}

bool MucomModule::Play() {
	if (!mucom) return false;
	if (mucom->Play(0) < 0) return false;
	playedFrames = 0;
	// フェーダー
	if (fade != nullptr) { delete fade;  fade = nullptr; }
	fade = new Fade(audioRate, GetLength() - 3);
	fade->enable = enableFade;

	return true;
}


void MucomModule::Close() {
	FreeMucom();
	FreeResultBuffer();
}

void MucomModule::FreeMucom() {
	if (!mucom) return;
	delete mucom;
	mucom = NULL;
}

void MucomModule::FreeResultBuffer() {
	if (!resultText) return;
	delete[] resultText;
	resultText = NULL;
}

const char* MucomModule::GetResult() {
	if (!resultText) return "";
	return resultText;
}

void MucomModule::Mix(short* data, int samples) {
	int buf[128];

	int index = 0;
	int skipSamples = 0;

	if (seekPosition >= 0) {
		int skipFrames = (seekPosition * audioRate) - (playedFrames);
		if (skipFrames > 0) {
			playedFrames += skipFrames;
			fade->Step(skipFrames);
		}
		// スキップするサンプル数 - 再生したサンプル数
		skipSamples = skipFrames * 2;
		if (skipSamples < 0) skipSamples = 0;
		seekPosition = -1;
	}

	// スキップ
	while (skipSamples > 0) {
		// 16サンプルを超過しない
		int s = skipSamples < 16 ? skipSamples : 16;
		mucom->RenderAudio(buf, s);
		skipSamples -= s;
	}



	while (samples > 0) {
		// 16サンプルを超過しない
		int s = samples < 16 ? samples : 16;
		mucom->RenderAudio(buf, s);

		for (int i = 0; i < s * 2;) {
			double vol = volume;
			vol *= fade->volume;
			fade->Step(1);

			// 1フレーム処理
			for (int j = 0; j < 2; i++, j++) {
				int v = (int)(buf[i] * vol);

				data[index] = v > 32767 ? 32767 : (v < -32768 ? -32768 : v);
				index++;
			}
			playedFrames++;
		}

		samples -= s;
	}
}
