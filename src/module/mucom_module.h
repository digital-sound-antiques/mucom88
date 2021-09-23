// MucomModule 
// BouKiCHi 2019

#ifndef _MUCOM_MODULE_H_
#define _MUCOM_MODULE_H_

#include "..\cmucom.h"
#include "../utils/mucomtag.h"
#include "../utils/fade.h"

class MucomModule  {
public:

    MucomModule();
    ~MucomModule();

    void SetWorkDir(const char* workingDirectory);

    bool Open(const char* songFilename);
    bool OpenMemory(uint8_t* data, int size, const char* path);
    bool Open(const char *workingDirectory, const char *songFilename);

    void SetRate(int rate);
    void SetPCM(const char *file);
    void SetVoice(const char *file);
    void SetVolume(double vol);

    int GetRate();

    /// <summary>
    /// �t�F�[�_�[�g�p�H
    /// </summary>
    void UseFader(bool enable);

    /// <summary>
    /// �f�t�H���g�̋ȕb��
    /// </summary>
    void SetDefaultLength(int length);

    /// <summary>
    /// �V�[�N(�b��) ������
    /// </summary>
    void Seek(int pos);

    /// <summary>
    /// �Đ��ʒu(�b��)
    /// </summary>
    int GetPosition();

    /// <summary>
    /// �I���H
    /// </summary>
    bool IsEnd();

    /// <summary>
    /// �Ȃ̒���(�b��) (#time�^�O���e or �f�t�H���g�l)
    /// </summary>
    int GetLength();

    /// <summary>
    /// �^�O�擾
    /// </summary>
    MucomTag *tag;

    void Mix(short *buffer, int samples);

    bool Play();
    void Close();
    const char *GetResult();
private:

    void LoadTag();

    int GetLengthFromTag();

    void LoadSongTag(const char* songFilename);

    int defaultLength;

    int length;

    double volume;
    bool enableFade;

    Fade *fade;

    const char *GetMucomMessage();
    void FreeMucom();
    void FreeResultBuffer();
    void AddResultBuffer(const char *text);

    CMucom *mucom;

    char *resultText;
    const char *pcmfile;
    const char *voicefile;
    const char *outfile;
    int audioRate;

    char directoryName[_MAX_PATH];
    char fileName[_MAX_PATH];

    int seekPosition;

    int driverMode;
    int cmpopt;

    int playedFrames;
};

#endif
