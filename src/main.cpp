
//
//	mucom : OpenMucom88 Command Line Tool
//			MUCOM88 by Yuzo Koshiro Copyright 1987-2019(C) 
//			Windows version by onion software/onitama 2018/11
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmucom.h"
#include "wavout.h"

// mainをSDL_mainにするために必要
#ifdef USE_SDL
#include <SDL.h>
#endif

//#define DEBUG_MUCOM

#define DEFAULT_OUTFILE "mucom88.mub"
#define DEFAULT_OUTWAVE "mucom88.wav"

#define RENDER_RATE 44100
#define RENDER_SECONDS 90

/*----------------------------------------------------------*/

static void usage1( void )
{
static const char *p[] = {
	"usage: mucom88 [options] [filename]",
	"       -p [filename] setload PCM file name",
	"       -v [filename] set load voice file name",
	"       -o [filename] set output MUB file name",
	"       -w [filename] set output WAV file name",
	"       -b [filename] set output log(.vgm/.s98) file name",
	"       -c [filename] compile mucom88 MML file name",
	"       -i [filename] info print mucom88 MML file name",
#ifdef MUCOM88WIN
	"       -a [filename] add external plugin",
#endif
	"       -e Use external ROM files",
	"       -s Use SCCI device",
	"       -k Skip PCM load",
	"       -x Recording mode",
	"       -d Dump used voice parameter",
	"       -l [n] Set recording lengh to n seconds ",
	"       -g Compile only",
	"       -z Original mode",
	"       -?, -h This help message ",
	NULL };
	int i;
	for (i = 0; p[i]; i++) {
		printf("%s\n", p[i]);
	}
}

/*----------------------------------------------------------*/

int main( int argc, char *argv[] )
{
	char a1,a2;
	int b,st;
	int cmpopt,ppopt,dumpopt;
	int scci_opt;
	char fname[1024];
	const char *pcmfile;
	const char *outfile;
	const char *wavfile;
	const char *logfile;
	const char *voicefile;
	const char *pluginfile;

#if defined(USE_SDL) && defined(_WIN32)
	freopen( "CON", "w", stdout );
	freopen( "CON", "w", stderr );
#endif

	//	check switch and prm

#ifdef DEBUG_MUCOM
	{
		CMucom mucom;

		mucom.Init();

		mucom.Reset(0);
		//mucom.ProcessFile("test.muc");
		//mucom.PrintInfoBuffer();

		mucom.LoadPCM();
		mucom.LoadMusic("test2.mub");
		mucom.Play(0);
		mucom.PrintInfoBuffer();

		//mucom.Reset(2);
		//mucom.LoadPCM();
		//mucom.CompileFile("sampl1.muc","test2.mub");

		puts(mucom.GetMessageBuffer());

		while (1) {
			Sleep(20);
		}


		return 0;
	}
#endif

	if (argc<2) { usage1();return -1; }

	st = 0; ppopt = 0; cmpopt = 0; scci_opt = 0; dumpopt = 0;
	pcmfile = MUCOM_DEFAULT_PCMFILE;
	outfile = DEFAULT_OUTFILE;
	wavfile = NULL;
	logfile = NULL;
	voicefile = NULL;
	pluginfile = NULL;
	fname[0] = 0;

	bool compile_only = false;
	bool original_mode = false;

	int song_length = 0;

	for (b=1;b<argc;b++) {
		a1=*argv[b];a2=tolower(*(argv[b]+1));
		if (a1!='-') {
			strcpy(fname,argv[b]);
		} else {
			switch (a2) {
			case 'p':
				pcmfile = argv[b + 1]; b++;
				ppopt = 0;
				break;
			case 'v':
				voicefile = argv[b + 1]; b++;
				break;
			case 'o':
				outfile = argv[b + 1]; b++;
				break;
			case 'w':
				wavfile = argv[b + 1]; b++;
				break;
			case 'b':
				logfile = argv[b + 1]; b++;
				break;			
			case 'a':
				pluginfile = argv[b + 1]; b++;
				break;
			case 'l':
				song_length = atoi(argv[b + 1]); b++;
				break;
			case 'c':
				cmpopt |= MUCOM_CMPOPT_COMPILE;
				break;
			case 'e':
				cmpopt |= MUCOM_CMPOPT_USE_EXTROM;
				break;
			case 'k':
				ppopt = 1;
				break;
			case 'i':
				cmpopt |= MUCOM_CMPOPT_INFO;
				break;
			case 's':
				scci_opt = 1;
				break;
			case 'x':
				cmpopt |= MUCOM_CMPOPT_STEP;
				break;
			case 'g':
				compile_only = true;
				break;
			case 'd':
				dumpopt = 1;
				break;
			case 'z':
				original_mode = true;
				break;
			case '?': case 'h':
				usage1(); 
				return -1;
			default:
				st=1;break;
			}
		}
	}

	if (st) { printf("#Illegal switch selected.\n");return 1; }
	if (fname[0]==0) { printf("#No file name selected.\n");return 1; }

	//		call main
	CMucom mucom;

	if (original_mode) {
		mucom.SetOriginalMode();
	}



	if (cmpopt & MUCOM_CMPOPT_STEP) {
		mucom.Init(NULL,cmpopt,RENDER_RATE);
	}
	else {
		if (scci_opt) {
			printf("Use SCCI.\n");
			mucom.Init(NULL, MUCOM_OPTION_SCCI | MUCOM_OPTION_FMMUTE, RENDER_RATE);
		}
		else {
			mucom.Init();
		}
	}

	// ログ設定
	if (logfile) {
		mucom.SetLogFilename(logfile);
	}

	// 初期化
	if (wavfile) {
		mucom.SetWavFilename(wavfile);
	}

	if (pluginfile) {
		printf("#Adding plugin %s.\n", pluginfile);
		int plgres = mucom.AddPlugins(pluginfile,0);
		if (plgres) {
			printf( "#Error adding plugin.(%d)\n", plgres );
		}
	}

	mucom.Reset(cmpopt);
	st = 0;

	if (cmpopt & MUCOM_CMPOPT_INFO) {
		mucom.ProcessFile(fname);
		mucom.PrintInfoBuffer();
		puts(mucom.GetMessageBuffer());
		return 0;
	}

	bool play_memory = false;
	const char* ext = strrchr(fname, '.');

	// mmlファイルはコンパイルをするようにする
	if (ext != NULL && strcmpi(ext, ".muc") == 0) cmpopt |= MUCOM_CMPOPT_COMPILE;

	if (cmpopt & MUCOM_CMPOPT_COMPILE) {
		if (ppopt == 0) {
			mucom.LoadPCM(pcmfile);
		}
		if (voicefile != NULL) {
			mucom.LoadFMVoice(voicefile);
		}
		if (mucom.CompileFile(fname, outfile) < 0) {
			st = 1;
		}
		play_memory = true;
	} else {
		if (mucom.LoadMusic(fname) < 0) {
			st = 1;
		}
	}

	if (st) {
		mucom.PrintInfoBuffer();
		puts(mucom.GetMessageBuffer());
		return st;
	}

	if (play_memory) { 
		mucom.PlayMemory(); 
	} else { 
		st = mucom.Play(0); 
	}

	if (st == 0) {
		if (dumpopt) {
			int i, max;
			max = mucom.GetUseVoiceMax();
			for (i = 0; i < max; i++) {
				mucom.DumpFMVoice(mucom.GetUseVoiceNum(i));
			}
		}
	}

	mucom.PrintInfoBuffer();
	puts(mucom.GetMessageBuffer());

	// コンパイルのみ
	if (play_memory && compile_only) return st;

	if (st == 0) {
		if (cmpopt & MUCOM_CMPOPT_STEP) {
			if (song_length <= 0) {
				const char* timetag = mucom.GetInfoBufferByName("time");
					song_length = atoi(timetag);
					if (song_length <= 0) {
						song_length = RENDER_SECONDS;
					}
			}
			if (logfile != NULL) printf("#Record to %s (%d sec).", logfile, song_length);
			if (wavfile != NULL) printf( "#Record to %s (%d sec).", wavfile, song_length );
			mucom.Record(song_length);

			// RecordWave(&mucom, wavfile, RENDER_RATE, song_length);
		}
		else {
			mucom.PlayLoop();
		}
	}

	return st;
}

