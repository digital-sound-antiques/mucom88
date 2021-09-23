#include <stdio.h>
#include <SDL.h>
#include "audiosdl.h"
#include "../module/mucom_module.h"
#include "../utils/pathutil.h"

class Player {
public:
    Player();
    ~Player();
    int Play(const char *filename);
    void Stop();

    void AudioCallback(void *mix, int size);

    AudioSdl *sdl;
    MucomModule *module;
private:
    bool EventCheck();
};

Player::Player() {
}

Player::~Player() {
}

bool Player::EventCheck() {
    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
        switch(evt.type) {
        case SDL_QUIT:
            return true;
        break;
        }
    }
    return false;
}

// オーディオコールバック
static void RunAudioCallback(void *CallbackInstance,void *MethodInstance);

static void RunAudioCallback(void *CallbackInstance,void *MethodInstance) {
    AudioCallback *acb = (AudioCallback *)CallbackInstance;
    ((Player *)MethodInstance)->AudioCallback(acb->mix, acb->size);
}

// オーディオ処理
void Player::AudioCallback(void *mix, int size) {
    module->Mix((short*)mix, size);
}

int Player::Play(const char *filename) {
    sdl = new AudioSdl();
    sdl->UserAudioCallback->Set(this,RunAudioCallback);
    module = new MucomModule();

    printf("File:%s\n", filename);

    auto path = new PathUtil(filename);

    const char* dir = path->GetDirectory();
    const char* name = path->GetFilename();
    module->SetWorkDir(dir);

#ifndef OPEN_FILE

    // メモリ上再生
    FILE *fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("File open error\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    uint8_t* data = new uint8_t[size + 1];
    fread(data, size, 1, fp);
    data[size] = 0;
    fclose(fp);

    // 開く
    bool r = module->OpenMemory(data, size, name);
#else 

    bool r = module->Open(name);
#endif

    puts(module->GetResult());

    if (!r) {
        printf("Compile error!\n");
        return -1;
    }

    // 曲名表示
    const char *title = module->tag->title.c_str();
    const char *composer = module->tag->composer.c_str();
    const char *author = module->tag->author.c_str();
    printf("Title:%s Composer:%s Author:%s\n", title, composer, author);

    // フェーダーとデフォルト秒数設定
    module->UseFader(true);
    // module->SetDefaultLength(180);

    int rate = module->GetRate();
    int length = module->GetLength();

    if (!r) return -1;
    r = module->Play();
    if (!r) return -1;

    // シーク
    //module->Seek(3);
    //module->Mix(NULL, 0);

    sdl->Open(rate);

    // イベントループ
    printf("Playing..\n");
    int pos = -1;

    while(pos < length) {
        if (EventCheck()) break;
        int newPos = module->GetPosition();
        if (pos != newPos) {
            pos = newPos;
            printf("%d / %d\r", pos, length);
        }
        SDL_Delay(20);
    }
    return 0;
}

void Player::Stop() {
    sdl->Close();
    delete sdl;
    sdl = NULL;

    module->Close();
    delete module;
    module = NULL;
}


int main(int argc, char *argv[]) {
#if defined(USE_SDL) && defined(_WIN32)
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr );

    char* res = setlocale(LC_ALL, ".UTF8");

#endif
    printf("MUCOM88 miniplay\n");
    if (argc < 2) {
        printf("usage miniplay <song.muc|song.mub>\n");
        return 0;
    }
    Player *p = new Player();
    p->Play(argv[1]);
    p->Stop();
    return 0;
}