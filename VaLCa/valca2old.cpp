#include "pch.h"
#include <inttypes.h>
#include "csaori.h"
#include "csaori_base.h"
#define NUM_META 5
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include <string>
typedef SSIZE_T ssize_t;
#include <map>
#include <vlc/vlc.h>
#include <filesystem>
#include <algorithm>
#include <time.h>
#define NUM_KEY 4
/*struct testInput {
    std::vector<string_t> args;

};
struct testOutput {
    string_t result;
    SAORIRESULT result_code;
};*/
struct mediaData {
    std::string path;
    std::string albumName;
    std::string title;
    std::string artist;
    int discNumber;
    int discTotal;
    int trackTotal;
    int64_t length;
    int trackNumber;
};
static mediaData initializer = { "__NULL__","__NULL__" ,"__NULL__" ,"__NULL__",0,0,0,0 };

bool operator <(const mediaData left, const mediaData right) {
    int compare = left.albumName.compare(right.albumName);
    if (compare == 0) {
        if (left.discNumber == right.discNumber)
            return left.trackNumber < right.trackNumber;
        else
            return left.discNumber < right.discNumber;
    }
    else {
        return compare < 0;
    }
}

std::string toStr(const char* str) {
    if (str == NULL) {
        return u8"__NULL__";
    }
    else {
        return str;
    }
}
int toInt(const char* str) {
    if (str == NULL) {
        return 0;
    }
    else {
        return atoi(str);
    }
}
class vlcPlayer {
private:
    //char* path;

    static libvlc_instance_t* instance;

    //static libvlc_media_t* media;
    //static libvlc_media_player_t* media_player;
    static int flug;
    static libvlc_media_list_t* mediaList;
    static libvlc_media_list_player_t* mediaListPlayer;
public:
    static int committing;
    static int playable;
    static std::vector<mediaData> mediaDataList;
    vlcPlayer();
    static int getFlug();
    static bool getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData);
    //static bool albumLoad(std::vector<std::string>& fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles);
    static bool albumLoad(std::vector<std::string>& fileNames);
    static int64_t getTime();
    static void initialize();
    static void setMedia(std::vector<std::string> fileNames);
    static void play();
    static bool isPlaying();
    static void stop();
    static void release();
    static void setMediaDataList(std::map<std::string, std::vector<mediaData>>& albumFiles);
    static mediaData getNowPlaying();
    static void getPathList(std::vector<std::string>& pathList);
    static void pauseToggle();
    static libvlc_state_t getState();
    static bool next();
    static bool previous();
    static int getNowPlayingMediaID();
};
libvlc_instance_t* vlcPlayer::instance;
int  vlcPlayer::playable;
int  vlcPlayer::committing;
std::vector<mediaData>  vlcPlayer::mediaDataList;
//libvlc_media_t* vlcPlayer::media;
//libvlc_media_player_t* vlcPlayer::media_player;
libvlc_media_list_player_t* vlcPlayer::mediaListPlayer;
int vlcPlayer::flug;
libvlc_media_list_t* vlcPlayer::mediaList;


void vlcPlayer::initialize() {
    instance = libvlc_new(0, NULL);
    mediaList = libvlc_media_list_new(instance);
    mediaListPlayer = libvlc_media_list_player_new(instance);
    flug = 0;
}
libvlc_state_t vlcPlayer::getState() {
    return libvlc_media_list_player_get_state(mediaListPlayer);
}
void DispStr(const char* name, const char* str) {
    if (str == NULL)
        printf("%s: NULL\n", name);
    else
        printf("%s: %s\n", name, str);
}
void vlcPlayer::setMedia(std::vector<std::string> fileNames) {
    libvlc_media_list_release(mediaList);
    mediaList = libvlc_media_list_new(instance);
    std::string* pathArray = fileNames.data();
    int arraySize = fileNames.size();
    /*if (flug != 0) {
        libvlc_media_list_player_release(mediaListPlayer);
    }*/
    for (int i = 0;i < arraySize;i++) {
        libvlc_media_t* mediaD = libvlc_media_new_path(instance, pathArray[i].c_str());
        libvlc_media_list_add_media(mediaList, mediaD);
        libvlc_media_release(mediaD);
    }
    libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);

    flug = 1;
}
vlcPlayer::vlcPlayer() {
}
int vlcPlayer::getFlug() {
    return flug;
}
int vlcPlayer::getNowPlayingMediaID() {
    libvlc_media_player_t* mediaPlayer = libvlc_media_list_player_get_media_player(mediaListPlayer);
    libvlc_media_t* mediaD = libvlc_media_player_get_media(mediaPlayer);
    return libvlc_media_list_index_of_item(mediaList, mediaD);
}
mediaData vlcPlayer::getNowPlaying() {
    int mediaID = getNowPlayingMediaID();
    //printf("NowPlaying mediaID:%d", mediaID);
    return mediaDataList[mediaID];
}

void vlcPlayer::play() {
    libvlc_media_list_player_play(mediaListPlayer);
    Sleep(100);
    libvlc_media_player_t* mediaPlayerD = libvlc_media_list_player_get_media_player(mediaListPlayer);
    libvlc_media_t* mediaD = libvlc_media_player_get_media(mediaPlayerD);
    const char* nowplaying = libvlc_media_get_meta(mediaD, libvlc_meta_Title);
    //DispStr("nowplaying", nowplaying);
}

void vlcPlayer::pauseToggle() {
    libvlc_state_t state = getState();
    //printf("state:%d\n", state);
    if (getFlug() && libvlc_state_t::libvlc_Playing == state) {

        libvlc_media_list_player_pause(mediaListPlayer);
    }
    else {
        vlcPlayer::play();
    }
}
int64_t vlcPlayer::getTime() {
    libvlc_media_player_t* mediaPlayerD = libvlc_media_list_player_get_media_player(mediaListPlayer);
    return libvlc_media_player_get_time(mediaPlayerD);
}
bool vlcPlayer::isPlaying() {
    //libvlc_media_list_player_get_media_player();
    return libvlc_state_t::libvlc_Ended != getState();

}
void vlcPlayer::stop() {
    if (getFlug()) {
        libvlc_media_list_player_stop(mediaListPlayer);
    }
}
void vlcPlayer::release() {
    libvlc_media_list_player_release(mediaListPlayer);
    libvlc_media_list_release(mediaList);
    libvlc_release(instance);
}
std::string timeStr(libvlc_time_t time) {
    char timeStr[19] = "";
    sprintf_s(timeStr, 19, "%lld", time);
    return toStr(timeStr);
}
//using namespace std::filesystem;
int idInStrings(std::string str, std::vector<std::string> list) {
    for (unsigned int i = 0;i < list.size();i++) {

        //printf(" %s== %s\n", list[i].c_str(), str.c_str());
        if (list[i] == str) {
            return i;
        }
    }
    //printf("はずれ\n");
    return -1;
}
bool filesInFolder(const char* input_path, std::vector<std::string>& fileNames, int mode) {
    using namespace std::filesystem;
    path path = u8path(input_path);
    directory_iterator iter(path), end;
    //std::vector<std::string> fileNames;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const directory_entry entry = *iter;
        //path entryPath=entry.path();
        std::string pathChar = entry.path().filename().u8string();
        if (pathChar[0] != u8'.') {
            if (is_directory(entry.path())) {
                if (mode == 1) filesInFolder(entry.path().u8string().c_str(), fileNames, mode);
            }
            else {
                fileNames.push_back(entry.path().u8string());
            }
        }

        //printf("%s\n", file_names.back().c_str());
    }
    if (err) {
        //printf("errorCode:%s\nerrorMessage:5s",err.value(),err.message());
        return false;
    }
    else {
        return true;
    }
}

bool vlcPlayer::next() {
    libvlc_state_t pState = getState();
    if (getFlug() && (pState == libvlc_Playing || pState == libvlc_Paused || pState == libvlc_Stopped || pState == libvlc_Ended)) {
        return libvlc_media_list_player_next(mediaListPlayer) == 0;
    }
    return false;
}
bool vlcPlayer::previous() {
    libvlc_state_t pState = getState();
    
    if (getFlug()&& (pState == libvlc_Playing || pState == libvlc_Paused || pState == libvlc_Stopped || pState == libvlc_Ended)) {
        return libvlc_media_list_player_previous(mediaListPlayer) == 0;
    }
    return false;
}

bool vlcPlayer::getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData) {
    //mediaData mediaData;
    libvlc_media_t* mediaD = libvlc_media_new_path(instance, path.u8string().c_str());
    if (libvlc_media_parse_with_options(mediaD, libvlc_media_fetch_local, 1000) == 0) {
        Sleep(80);
        mediaData.title = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Title));
        mediaData.artist = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Artist));
        mediaData.trackTotal = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_TrackTotal));
        mediaData.length = libvlc_media_get_duration(mediaD);
        mediaData.path = path.u8string();
        mediaData.albumName = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Album));
        mediaData.trackNumber = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_TrackNumber));
        mediaData.discNumber = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_DiscNumber));
        mediaData.discTotal = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_DiscTotal));
        return true;
    }
    else {
        return false;
    }
}
//bool vlcPlayer::albumLoad(std::vector<std::string> &fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles) {//----------第二
bool vlcPlayer::albumLoad(std::vector<std::string>& fileNames) {//----------第二

    //printf("===albumLoad===\n");
    std::vector <libvlc_media_t*> mediaV;
    int k = 0;
    if (fileNames.size() < 1) {
        return false;
    }
    for (unsigned int i = 0;i < fileNames.size();i++) {
        libvlc_media_t* media = libvlc_media_new_path(instance, fileNames[i].c_str());
        mediaV.push_back(media);

        if (libvlc_media_parse_with_options(mediaV.back(), libvlc_media_fetch_local, 1000) != 0) {
            fileNames.erase(fileNames.begin() + i);
            mediaV.pop_back();
            libvlc_media_release(media);
            i--;
            //printf("couldn't be opened\n");
        }
        //printf("load%d\n", i);
    }
    //Sleep( 100 + 30 * fileNames.size());
    if (mediaV.size() < 1) return false;
    int count;
    Sleep(100);
    while (libvlc_media_get_meta(mediaV[mediaV.size() - 1], libvlc_meta_TrackNumber) == NULL) {
        //Sleep(120 + 100 * fileNames.size());
        Sleep(120);
        count++;
        if (count > fileNames.size()) {
            break;
        }
    }
    Sleep(50);
    //std::vector <mediaData> meta(mediaV.size());
    //std::vector <std::string> albumNames;
    int n = -1;
    mediaDataList.clear();
    for (unsigned int i = 0;i < mediaV.size();i++) {
        mediaData mD;
        mD.title = toStr(libvlc_media_get_meta(mediaV[i], libvlc_meta_Title));
        mD.artist = toStr(libvlc_media_get_meta(mediaV[i], libvlc_meta_Artist));
        mD.trackTotal = toInt(libvlc_media_get_meta(mediaV[i], libvlc_meta_TrackTotal));
        mD.discNumber = toInt(libvlc_media_get_meta(mediaV[i], libvlc_meta_DiscNumber));
        mD.discTotal = toInt(libvlc_media_get_meta(mediaV[i], libvlc_meta_DiscTotal));
        mD.length = libvlc_media_get_duration(mediaV[i]);
        mD.path = fileNames[i];
        mD.albumName = toStr(libvlc_media_get_meta(mediaV[i], libvlc_meta_Album));
        mD.trackNumber = toInt(libvlc_media_get_meta(mediaV[i], libvlc_meta_TrackNumber));
        mediaDataList.push_back(mD);
        libvlc_media_release(mediaV[i]);
    }
    std::sort(mediaDataList.begin(), mediaDataList.end());
    return true;
}
void vlcPlayer::setMediaDataList(std::map<std::string, std::vector<mediaData>>& albumFiles) {
    for (auto itr = albumFiles.begin(); itr != albumFiles.end(); ++itr) {
        for (unsigned int i = 0;i < itr->second.size();i++) {
            mediaDataList.push_back(itr->second[i]);
        }
    }
}
void vlcPlayer::getPathList(std::vector<std::string>& pathList) {
    for (unsigned int i = 0;i < mediaDataList.size();i++) {
        pathList.push_back(mediaDataList[i].path);
    }
}



void DispTitleList() {
    for (unsigned int i = 0;i < vlcPlayer::mediaDataList.size();i++) {
        mediaData mediaData = vlcPlayer::mediaDataList[i];
        printf("%d-%d:%s\n", mediaData.discNumber, mediaData.trackNumber, mediaData.title.c_str());
    }
}

bool loadPath(std::filesystem::path path) {

    bool playFlug = true;
    std::vector<std::string> pathList;
    if (is_directory(path)) {
        std::vector <std::string> fileNames;
        //std::map<std::string, std::vector<mediaData>> albumMap;
        filesInFolder(path.u8string().c_str(), fileNames, 1);
        //if (vlcPlayer::albumLoad(fileNames, albumMap)) {
        if (vlcPlayer::albumLoad(fileNames)) {
            vlcPlayer::getPathList(pathList);
        }
        else {
            return false;
        }
    }
    else {
        pathList.push_back(path.u8string().c_str());
        mediaData mediaData;
        if (!vlcPlayer::getMediaDataWithPath(path, mediaData)) {
            return false;
        }
        else {
            vlcPlayer::mediaDataList.clear();
            vlcPlayer::mediaDataList.push_back(mediaData);

        }
    }
    vlcPlayer::setMedia(pathList);
    return true;
}
std::string execute(std::vector<std::string>inputs) {
    clock_t start = clock();
    //printf("====EXEC====");
    int i = 0;
    bool outputFlug = true;
    std::string status = u8"OK";

    while (vlcPlayer::committing == 1) {
        if (i > 300) {
            //printf("TimeOut");
            return "error:timeout\n";
        }
        Sleep(10);
        i++;
    }
    vlcPlayer::committing = 1;
    if (inputs[0] == u8"stop") {
        vlcPlayer::stop();
    }
    else if (inputs[0] == u8"pauseToggle") {
        vlcPlayer::pauseToggle();
    }
    else if (inputs[0] == u8"previous") {
        if (!vlcPlayer::previous()) {
            status = u8"error";
        }
        
        Sleep(80);
    }
    else if (inputs[0] == u8"next") {
        if (!vlcPlayer::next()) {
            status = u8"error";
        }
        Sleep(80);
    }
    else if (inputs[0] == u8"now") {
        if (!vlcPlayer::getFlug()) {
            status = u8"error";
        }
        Sleep(80);
    }
    else {
        std::string path_str;
        int mode = 0;
        if (inputs[0] == u8"play" || inputs[0] == u8"play2") {
            path_str = inputs[1];
            if (inputs[0] == u8"play2") mode = 1;
        }
        else path_str = inputs[0];
        bool ended = vlcPlayer::getState() == libvlc_Ended && vlcPlayer::getNowPlayingMediaID() + 1 == vlcPlayer::mediaDataList.size();
        if (vlcPlayer::getFlug() == 0 || mode == 0 || ended) {
            using namespace std::filesystem;
            const path path = u8path(path_str.c_str());

            if (loadPath(path)) {
                if (vlcPlayer::getFlug != 0) {
                    vlcPlayer::stop();
                }
                vlcPlayer::play();
                //printf("====DISPTITLELIST====\n");
                //DispTitleList();
                //printf("^^^^^DISPTITLELIST^^^^\n");
            }
            else {
                status = "error:couldn't load the file(s)";
                outputFlug = false;
            }

        }
        else {
            status = "error:playing now!";
        }

    }
    std::string output;
    libvlc_state_t playerState = vlcPlayer::getState();
    //char buffer[50]; 
    if (playerState == NULL) {
        //printf("playerState==NULL\n");
    }
    //printf("---OUTPUT----\n");
    output = status + u8"\n";
    if (vlcPlayer::getFlug()) {
        output += std::to_string(playerState) + u8"\n";
        bool x = playerState == libvlc_Opening || playerState == libvlc_Playing || playerState == libvlc_Paused;
        mediaData mediaData = vlcPlayer::getNowPlaying();
        output += mediaData.title + u8"\n";
        output += mediaData.albumName + u8"\n";
        output += mediaData.artist + u8"\n";
        output += mediaData.path + u8"\n";
        output += std::to_string(mediaData.trackNumber) + u8"\n";
        output += std::to_string(mediaData.trackTotal) + u8"\n";
        output += std::to_string(mediaData.discNumber) + u8"\n";
        output += std::to_string(mediaData.discTotal) + u8"\n";
        output += timeStr(mediaData.length) + u8"\n";
        output += timeStr(vlcPlayer::getTime()) + u8"\n";
        output += std::to_string(vlcPlayer::getNowPlayingMediaID()) + u8"\n";
        output += std::to_string(vlcPlayer::mediaDataList.size()) + u8"\n";
    }
    else {
        output += u8"No Media";
    }
    
    //printf("%s", output.c_str());
    clock_t end = clock();
    const double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
    //printf("time %lf[ms]\n", time);
    vlcPlayer::committing = 0;
    return output;
}
bool loadex() {
    vlcPlayer::committing = 1;
    //playerインスタンスの作成
    vlcPlayer::initialize();
    vlcPlayer::committing = 0;
    return true;
}
bool unloadex() {
    if (vlcPlayer::isPlaying())
        vlcPlayer::stop();
    vlcPlayer::release();
    return true;
}

bool CSAORI::load() {
    return loadex();
}
bool CSAORI::unload() {
    return unloadex();
}
void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
    std::vector<std::string> inputs;
    for (unsigned int i = 0;i < in.args.size();i++) {
        std::string arg=SAORI_FUNC::UnicodeToMultiByte(in.args[0], SAORI_FUNC::StringtoCodePage("utf-8"));
        inputs.push_back(arg);
    }
    std::string output=execute(inputs);
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
            Sleep(10);
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
