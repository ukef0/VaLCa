#include "pch.h"

#include "csaori.h"
#include "vlcPlayer.h"

//std::mutex mtx_;

//#define OpenFileMappingW OpenFileMapping
/*struct testInput {
    std::vector<string_t> args;

};
struct testOutput {
    string_t result;
    SAORIRESULT result_code;
};*/





bool CSAORI::load() {
    return vlc.loadex();
}
bool CSAORI::unload() {
    return vlc.unloadex();
}
void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
    std::vector<std::string> inputs;
    for (unsigned int i = 0;i < in.args.size();i++) {
        std::string arg=SAORI_FUNC::UnicodeToMultiByte(in.args[i], SAORI_FUNC::StringtoCodePage("utf-8"));
        inputs.push_back(arg);
    }
    std::string output=vlc.execute(inputs);
    out.result = SAORI_FUNC::MultiByteToUnicode(output, SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
    out.result_code = SAORIRESULT_OK;
}

/*void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
    //const char* path = u8"C:\\Users\\shono.f\\Desktop\\sample.m4a";
    //const char* path = in.args[0].c_str();
    const std::string argument = SAORI_FUNC::UnicodeToMultiByte(in.args[0], SAORI_FUNC::StringtoCodePage("utf-8"));
    int playFlug = 0;
    //string_t mode = in.args[0];
    out.result_code = SAORIRESULT_OK;
    out.result = SAORI_FUNC::numToString(0) + L"\n";
    if (in.args[0] == L"stop") {
        if (vlcPlayer::getFlug()) {
            vlcPlayer::stop();
        }
    }
    else if (in.args[0] == L"pauseToggle") {
        vlcPlayer::pauseToggle();
    }else if (in.args[0] == L"play2") {
        const std::string path = SAORI_FUNC::UnicodeToMultiByte(in.args[1], SAORI_FUNC::StringtoCodePage("utf-8"));
        if (vlcPlayer::getFlug == 0|| !vlcPlayer::isPlaying()) {
            vlcPlayer::setMedia(path.c_str());
            vlcPlayer::play();
            playFlug = 1;
        }else {
            
            out.result = L"0\n";
            for (int i = 0;i < NUM_META;i++) {
                out.result += SAORI_FUNC::MultiByteToUnicode(vlcPlayer::meta[i], SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
            }
            std::string timeResult = timeStr(vlcPlayer::getTime());
            out.result += SAORI_FUNC::MultiByteToUnicode(timeResult, SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
        }
    }else{
        
        //const std::string path = SAORI_FUNC::UnicodeToMultiByte(in.args[1], SAORI_FUNC::StringtoCodePage("utf-8"));
        vlcPlayer::setMedia(argument.c_str());
        vlcPlayer::play();
        playFlug = 1;
    }

    
    if (vlcPlayer::isPlaying()&&playFlug) {
        out.result =SAORI_FUNC::numToString(1) + L"\n";
        //out.result += SAORI_FUNC::MultiByteToUnicode(vlcPlayer::meta[0], SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
        //out.result += SAORI_FUNC::MultiByteToUnicode(vlcPlayer::meta[1], SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
        for (int i = 0;i < NUM_META ;i++) {
            out.result += SAORI_FUNC::MultiByteToUnicode(vlcPlayer::meta[i], SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";
        }
        std::string timeResult = timeStr(vlcPlayer::getTime());
        out.result += SAORI_FUNC::MultiByteToUnicode(timeResult, SAORI_FUNC::StringtoCodePage("utf-8")) + L"\n";

    }
}*/
/*
void keyVerify(bool* key) {
    key[0] = (GetKeyState(VK_SPACE) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[1] = (GetKeyState(VK_LEFT) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[2] = (GetKeyState(VK_RIGHT) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[3] = (GetKeyState(0x31) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
}
void keyCopy(bool* key, bool* key2) {
    for (int i = 0;i < NUM_KEY;i++) {
        key2[0] = key[0];
    }
}
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    loadex();
    unloadex();
    if (1) {
        // char input_path[200];
        //std::string input_path = u8"C:\\Users\\shono.f\\Desktop\\私猫なのー？long.m4a";
        //std::string input_path = u8"F:\\QuickTime Player\\SR320";
        //std::string input_path=u8"C:\\Users\\shono.f\\Desktop\\H?l?ne De Montgeroult";
        //std::string input_path=u8"C:\\Users\\shono.f\\Desktop\\frenchsuite";
        //std::string input_path = u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Glenn Gould\\J.S.Bach_ Well-Tempered Clavier, Book 1, BWV 854-861";
        //std::string input_path=u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Glenn Gould\\Bach_ Well-Tempered Clavier, Book 2, BWV 886-893";
        string_t input_path = L"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Edna Stern\\H?l?ne De Montgeroult";
        loadex();
        std::vector<string_t>inputs;
        inputs.push_back((input_path));
        std::vector<string_t>outputs;
        //CSAORIInput* exinput;
        //exinput.args = inputs;
        testInput testIn;
        testIn.args = inputs;
        testOutput testOut;
        exec(testIn,testOut);

        //vlcPlayer Player2(u8"C:\\Users\\shono.f\\Desktop\\SRパレット.m4a");
        //vlcPlayer::play();

        //while (vlcPlayer::isPlaying()){

        bool key[NUM_KEY];
        std::vector<string_t> keyCommand(NUM_KEY);
        keyCommand[0] = L"pauseToggle";
        keyCommand[1] = L"previous";
        keyCommand[2] = L"next";
        keyCommand[3] = L"F:\\QuickTime Player\\SR320";

        keyVerify(key);
        bool newKey[NUM_KEY];
        keyCopy(key, newKey);
        std::vector<string_t> in;
        while (1) {
            tSleep(10);
            keyVerify(newKey);
            for (int i = 0;i < NUM_KEY;i++) {
                if (newKey[i] && newKey[i] != key[i]) {
                    in.clear();
                    in.push_back(keyCommand[i]);
                    testInput tesin;
                    tesin.args = in;
                    clock_t start = clock();
                    testOutput tesout;
                    exec(tesin,tesout);
                    clock_t end = clock();
                    const double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
                    printf("time %lf[ms]\n", time);
                    continue;
                }
            }

            keyCopy(newKey, key);

        }
        unloadex();
    }


    return 0;
}*/
