#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include <string>
#include <fstream>
#include <iostream>
//typedef SSIZE_T ssize_t;
#include <map>

#include <filesystem>
#include <algorithm>
#include <time.h>
#include <random>
#include <regex>
//#undef OpenFileMapping 
#include <windows.h>
#include <mutex>
#include <vlc/vlc.h>
//#include "VaLCa_util.h"
#include "vlcPlayer.h"
//using namespace VaLCaFunc;


//



    std::string mediaDataStr(mediaData mediaData);
    std::string myGetCurrentDirectry();
    std::vector< std::string > myMatch(std::string const& text, std::regex const& re);
    std::string getHwnd(std::string fmo);
    bool send_sstp(const std::string& str, void* hwnd);
    void sstpNotify(std::string event, std::vector<std::string> references);
    //void testsstp(std::vector<std::string> references);
    void testsstp(std::string reference);
    void OnVaLCaSSTP(std::string command);
    void OnVaLCaPass();
    void OnVaLCaTest();
    void OnVaLCaLoop();
    void OnVaLCaFinish();
    int idInStrings(std::string str, std::vector<std::string> list);
    bool filesInFolder(const char* input_path, std::vector<std::string>& fileNames, int mode);
    std::vector<std::string> loadExList();
    std::filesystem::path getDllPath();
    void tSleep(int x);
    std::string toStr(const char* str);
    int toInt(const char* str);

    std::string timeStr(libvlc_time_t time);




void vlcPlayer::CallbackNextFunc(const libvlc_event_t* event, void* arg) {
    reinterpret_cast<vlcPlayer*>(arg)->nextHundler(event);
}
void vlcPlayer::CallbackPlayedFunc(const libvlc_event_t* event, void* arg) {
    reinterpret_cast<vlcPlayer*>(arg)->playedHundler(event);
}

bool vlcPlayer::isEnded() {
    mtxVLC_.lock();
    unsigned int x = vlcPlayer::mediaDataList.size();
    mtxVLC_.unlock();
    bool cond1 = libvlc_state_t::libvlc_Ended == getState();
    bool cond2 = getNowPlayingMediaID() == x - 1;
    return cond1 && cond2;
}
void vlcPlayer::setCommitting(int x) {
    mtxVLCcommit_.lock();
    committing = x;
    mtxVLCcommit_.unlock();
}
int vlcPlayer::getCommitting() {
    mtxVLCcommit_.lock();
    int x = committing;
    mtxVLCcommit_.unlock();
    return x;
}

void vlcPlayer::initialize() {
    char buffer[MY_MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MY_MAX_PATH);

    //std::filesystem::path mpath = std::filesystem::u8path(buffer);
    //mpath /= u8"VaLCa/";
    PCWSTR x = L"F:\\ssp\\VaLCa";
    //PCWSTR x = L"F:\\ssp\\VaLCa";
    //string_t rq = SAORI_FUNC::MultiByteToUnicode(mpath.u8string(), SAORI_FUNC::StringtoCodePage("utf-8"));
    //std::string sjbuf = SAORI_FUNC::UnicodeToMultiByte(rq, CP_SJIS);
    //mpath.u8string().c_str()
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    AddDllDirectory(x);
    //SetCurrentDirectoryW(x);


}

void vlcPlayer::loopToggle() {
    //printf("looptoggle\n");
    if (getPlaybackMode() != libvlc_playback_mode_loop)
        setPlaybackMode(libvlc_playback_mode_loop);
    else
        setPlaybackMode(libvlc_playback_mode_default);
}
void vlcPlayer::repeatToggle() {
    //printf("repeattoggle\n");
    if (getPlaybackMode() != libvlc_playback_mode_repeat)
        setPlaybackMode(libvlc_playback_mode_repeat);
    else
        setPlaybackMode(libvlc_playback_mode_default);
}
void vlcPlayer::setRandomMode(int x) {
    mtxVLC_.lock();
    randomMode = x;
    mtxVLC_.unlock();
}
void vlcPlayer::randomToggle() {
    mtxVLC_.lock();
    randomMode = 1 - randomMode;
    mtxVLC_.unlock();
}
int vlcPlayer::getRandomMode() {
    mtxVLC_.lock();
    int x = randomMode;
    mtxVLC_.unlock();
    return x;
}
void vlcPlayer::setPlaybackMode(libvlc_playback_mode_t x) {
    mtxVLC_.lock();
    playbackMode = x;
    libvlc_media_list_player_set_playback_mode(mediaListPlayer, playbackMode);
    mtxVLC_.unlock();
}

libvlc_playback_mode_t vlcPlayer::getPlaybackMode() {
    mtxVLC_.lock();
    libvlc_playback_mode_t x = playbackMode;
    mtxVLC_.unlock();
    return x;
}
libvlc_state_t vlcPlayer::getState() {
    mtxVLC_.lock();
    libvlc_state_t x = libvlc_media_list_player_get_state(mediaListPlayer);
    mtxVLC_.unlock();
    return x;
}
void DispStr(const char* name, const char* str) {
    if (str == NULL)
        printf("%s: NULL\n", name);
    else
        printf("%s: %s\n", name, str);
}
void vlcPlayer::setMedia() {
    mtxVLC_.lock();
    libvlc_media_list_release(mediaList);
    mediaList = libvlc_media_list_new(instance);
    for (unsigned int i = 0;i < mediaDataList.size();i++) {
        libvlc_media_t* mediaD = libvlc_media_new_path(instance, mediaDataList[i].path.c_str());
        libvlc_media_list_add_media(mediaList, mediaD);
        libvlc_media_release(mediaD);
    }
    libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
    mtxVLC_.unlock();
    setFlug(1);

}
vlcPlayer::vlcPlayer() {
    std::vector<std::string> exList = { ".mp3",".m4a",".wav",".ogg",".flac",".alac",".wma",".aac",".gsm",".au",".aif" };
    std::vector<std::string> exLoaded = loadExList();
    if (exLoaded.size() > 0) {
        exList = exLoaded;
    }
    //mtxVLC_.lock();
    errorMess = "";
    extensionMode = 1;
    extensionList = exList;
    played = 0;
    instance = libvlc_new(0, NULL);
    mediaList = libvlc_media_list_new(instance);
    mediaListPlayer = libvlc_media_list_player_new(instance);
    //mtxVLC_.unlock();
    flug = 0;
    randomMode = 0;
    playbackMode = libvlc_playback_mode_default;
    committing = 0;
    //mtxVLC_.lock();
    eventManager = libvlc_media_list_player_event_manager(mediaListPlayer);
    libvlc_event_attach(eventManager, libvlc_MediaListPlayerPlayed, vlcPlayer::CallbackPlayedFunc,this);
    libvlc_event_attach(eventManager, libvlc_MediaListPlayerNextItemSet, vlcPlayer::CallbackNextFunc, this);

}
int vlcPlayer::getFlug() {
    mtxVLCflg_.lock();
    int x = flug;
    mtxVLCflg_.unlock();
    return x;
}
void vlcPlayer::setFlug(int x) {
    mtxVLCflg_.lock();
    flug = x;
    mtxVLCflg_.unlock();
}
int vlcPlayer::getNowPlayingMediaID() {
    mtxVLC_.lock();
    libvlc_media_player_t* mediaPlayer = libvlc_media_list_player_get_media_player(mediaListPlayer);
    libvlc_media_t* mediaD = libvlc_media_player_get_media(mediaPlayer);
    int x = libvlc_media_list_index_of_item(mediaList, mediaD);
    mtxVLC_.unlock();
    return x;
}
mediaData vlcPlayer::getNowPlaying() {
    int mediaID = getNowPlayingMediaID();
    //printf("NowPlaying mediaID:%d", mediaID);
    mtxVLC_.lock();
    mediaData x = mediaDataList[mediaID];
    mtxVLC_.unlock();
    return x;
}

void vlcPlayer::play() {
    mtxVLC_.lock();
    played = 0;
    libvlc_media_list_player_play(mediaListPlayer);
    /*tSleep(100);
    libvlc_media_player_t* mediaPlayerD = libvlc_media_list_player_get_media_player(mediaListPlayer);
    libvlc_media_t* mediaD = libvlc_media_player_get_media(mediaPlayerD);
    const char* nowplaying = libvlc_media_get_meta(mediaD, libvlc_meta_Title);*/
    mtxVLC_.unlock();
    //DispStr("nowplaying", nowplaying);
}

void vlcPlayer::pauseToggle() {
    libvlc_state_t state = getState();
    //printf("state:%d\n", state);
    int flug = getFlug();
    mtxVLC_.lock();
    if (flug && libvlc_state_t::libvlc_Playing == state) {

        libvlc_media_list_player_pause(mediaListPlayer);
        mtxVLC_.unlock();
    }
    else {
        mtxVLC_.unlock();
        vlcPlayer::play();
    }
}
int64_t vlcPlayer::getTime() {
    mtxVLC_.lock();
    libvlc_media_player_t* mediaPlayerD = libvlc_media_list_player_get_media_player(mediaListPlayer);
    int64_t x = libvlc_media_player_get_time(mediaPlayerD);
    mtxVLC_.unlock();
    return x;
}
bool vlcPlayer::isPlaying() {
    //libvlc_media_list_player_get_media_player();
    return libvlc_state_t::libvlc_Ended != getState();

}
void vlcPlayer::stop() {
    if (getFlug()) {
        mtxVLC_.lock();
        libvlc_media_list_player_stop(mediaListPlayer);
        mtxVLC_.unlock();
    }
}
bool vlcPlayer::playItemAtIndex(int id) {
    bool res = false;
    if (getFlug()) {
        mtxVLC_.lock();
        if (id < (signed)mediaDataList.size()) {
            played = 0;
            if (libvlc_media_list_player_play_item_at_index(mediaListPlayer, id) == 0)
                res = true;
        }
        mtxVLC_.unlock();
    }
    return res;
}
void vlcPlayer::release() {
    mtxVLC_.lock();
    libvlc_event_detach(eventManager, libvlc_MediaListPlayerPlayed, vlcPlayer::CallbackPlayedFunc, this);
    libvlc_event_detach(eventManager, libvlc_MediaListPlayerNextItemSet, vlcPlayer::CallbackNextFunc, this);
    libvlc_media_list_player_release(mediaListPlayer);
    libvlc_media_list_release(mediaList);
    libvlc_release(instance);
    mtxVLC_.unlock();
}
bool vlcPlayer::next() {
    //sstpThread::setChangeByMe(1);
    mtxVLC_.lock();
    int x = libvlc_media_list_player_next(mediaListPlayer);
    mtxVLC_.unlock();
    return  x == 0;
}
bool vlcPlayer::previous() {
    //sstpThread::setChangeByMe(1);
    mtxVLC_.lock();
    played = 0;
    int x = libvlc_media_list_player_previous(mediaListPlayer);
    mtxVLC_.unlock();
    return x == 0;
}

bool vlcPlayer::getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData) {
    //mediaData mediaData;
    mtxVLC_.lock();
    std::string extension = path.extension().u8string();
    std::transform(extension.cbegin(), extension.cend(), extension.begin(), tolower);
    bool extensionFound = std::find(extensionList.begin(), extensionList.end(), extension) != extensionList.end();
    if (extensionMode && !extensionFound) {
        mtxVLC_.unlock();
        return false;

    }

    libvlc_media_t* mediaD = libvlc_media_new_path(instance, path.u8string().c_str());
    if (libvlc_media_parse_with_options(mediaD, libvlc_media_fetch_local, 1000) == 0) {
        while (!libvlc_media_is_parsed(mediaD))
            tSleep(100);
        mediaData.title = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Title));
        mediaData.artist = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Artist));
        mediaData.trackTotal = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_TrackTotal));
        mediaData.length = libvlc_media_get_duration(mediaD);
        mediaData.path = path.u8string();
        mediaData.albumName = toStr(libvlc_media_get_meta(mediaD, libvlc_meta_Album));
        mediaData.trackNumber = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_TrackNumber));
        mediaData.discNumber = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_DiscNumber));
        mediaData.discTotal = toInt(libvlc_media_get_meta(mediaD, libvlc_meta_DiscTotal));
        mtxVLC_.unlock();
        return true;
    }
    else {
        mtxVLC_.unlock();
        return false;
    }
}
//bool vlcPlayer::albumLoad(std::vector<std::string> &fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles) {//----------第二
bool vlcPlayer::albumLoad(std::vector<std::string>& fileNames, int addFlug = 0) {//----------第二
    //mtxVLC_.lock();
    //printf("===albumLoad===\n");
    std::vector <libvlc_media_t*> mediaV;
    int k = 0;
    if (fileNames.size() < 1) {
        //mtxVLC_.unlock();
        return false;
    }
    mtxVLC_.lock();
    for (int i = 0;i < (signed)fileNames.size();i++) {
        std::filesystem::path fpath = std::filesystem::u8path(fileNames[i].c_str());
        std::string extension = fpath.extension().u8string();
        std::transform(extension.cbegin(), extension.cend(), extension.begin(), tolower);
        bool extensionFound = std::find(extensionList.begin(), extensionList.end(), extension) != extensionList.end();
        if (extensionMode && !extensionFound) {

            fileNames.erase(fileNames.begin() + i);
            i--;
            continue;
        }
        libvlc_media_t* media = libvlc_media_new_path(instance, fileNames[i].c_str());
        mediaV.push_back(media);

        if (libvlc_media_parse_with_options(mediaV.back(), libvlc_media_fetch_local, 3000) != 0) {
            fileNames.erase(fileNames.begin() + i);
            mediaV.pop_back();
            libvlc_media_release(media);
            i--;
            //printf("couldn't be opened\n");
        }
        //printf("load%d\n", i);
    }

    tSleep(120);
    unsigned int count = 0;
    if (mediaV.size() < 1) {
        mtxVLC_.unlock();
        return false;
    }

    while (!libvlc_media_is_parsed(mediaV[mediaV.size() - 1]) || libvlc_media_get_duration(mediaV[mediaV.size() - 1]) <= 10) {
        tSleep(80);
        count++;
        if (count > mediaV.size() * 10) {

            break;
        }
    }
    tSleep(80);
    //int n = -1;
    for (int i = 0;i < (signed)mediaV.size();i++) {
        if (libvlc_media_get_duration(mediaV[i]) == NULL || libvlc_media_get_duration(mediaV[i]) < 10) {
            //mtxVLC_.unlock();
            std::string mess = (u8"erase" + std::to_string(i));
            //std::thread sstpThread(testsstp,mess);
            //sstpThread.detach();
            //mtxVLC_.lock();
            mediaV.erase(mediaV.begin() + i);
            i--;
        }
    }
    if (mediaV.size() < 1) {
        mtxVLC_.unlock();
        return false;
    }
    //mtxVLC_.lock();
    if (addFlug == 0) {
        mediaDataList.clear();
    }

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
    //sortMediaDataList();
    mtxVLC_.unlock();
    return true;

}

bool vlcPlayer::addMediaInList(std::vector<std::filesystem::path> paths) {
    mtxVLC_.lock();
    int mediaDataSize = mediaDataList.size();
    mtxVLC_.unlock();
    int res = 0;
    for (unsigned int i = 0;i < paths.size();i++) {
        if (loadPath(paths[i], 1)) {
            res = 1;
        }
    }
    if (res == 1) {

        sortMediaDataList(mediaDataSize, 0);
        mtxVLC_.lock();
        for (unsigned int i = mediaDataSize;i < mediaDataList.size();i++) {
            libvlc_media_t* mediaD = libvlc_media_new_path(instance, mediaDataList[i].path.c_str());
            libvlc_media_list_add_media(mediaList, mediaD);
            libvlc_media_release(mediaD);
        }
        mtxVLC_.unlock();
        return true;
    }
    else {
        return false;
    }

}
void vlcPlayer::sortMediaDataList(int start = 0, int end = 0) {
    mtxVLC_.lock();
    if (randomMode == 1) {
        std::mt19937 get_rand_mt((unsigned int)time(NULL));
        std::shuffle(mediaDataList.begin() + start, mediaDataList.end() - end, get_rand_mt);
    }
    else {
        std::sort(mediaDataList.begin() + start, mediaDataList.end() - end);
        //printf("===normalsorting===\n");
    }
    mtxVLC_.unlock();
}
void vlcPlayer::setMediaDataList(std::map<std::string, std::vector<mediaData>>& albumFiles) {
    mtxVLC_.lock();
    for (auto itr = albumFiles.begin(); itr != albumFiles.end(); ++itr) {
        for (unsigned int i = 0;i < itr->second.size();i++) {
            mediaDataList.push_back(itr->second[i]);
        }
    }

    mtxVLC_.unlock();
}
void vlcPlayer::getPathList(std::vector<std::string>& pathList) {
    mtxVLC_.lock();
    for (unsigned int i = 0;i < mediaDataList.size();i++) {
        pathList.push_back(mediaDataList[i].path);
    }
    mtxVLC_.unlock();
}



std::string vlcPlayer::playerDataStr() {
    std::string output = "";
    output += std::to_string(getState()) + u8"\n";
    output += timeStr(getTime()) + u8"\n";
    output += std::to_string(getNowPlayingMediaID()) + u8"\n";
    output += std::to_string(mediaDataList.size()) + u8"\n";
    output += std::to_string(getPlaybackMode()) + u8"\n";

    return output;
}
bool vlcPlayer::loadPath(std::filesystem::path path, int addFlug = 0) {
    bool playFlug = false;
    std::vector<std::string> pathList;
    if (!exists(path)) {
        errorMess += u8"file not found\n";
        return false;
    }

    //「予定」ファイルが非存在の時、上位フォルダ内で類似名のファイルを走査
    if (is_directory(path)) {
        std::vector <std::string> fileNames;
        filesInFolder(path.u8string().c_str(), fileNames, 1);
        if (albumLoad(fileNames, addFlug)) {
            playFlug = true;
        }
        else {
            errorMess += u8"vlc couldn't load directory\nmay be there are n't files?\n";
            return false;
        }
    }
    else {
        mediaData mediaData;
        if (!getMediaDataWithPath(path, mediaData)) {
            errorMess += u8"vlc couldn't load the file\n";
            return false;
        }
        else {
            // mtxVLC_.lock();
            if (addFlug == 0) {
                mediaDataList.clear();
            }
            mediaDataList.push_back(mediaData);
            //mtxVLC_.unlock();
        }
    }

    return true;
}
std::string vlcPlayer::execute(std::vector<std::string>inputs) {
    //std::lock_guard<std::mutex> lock(mtx_);
    int i = 0;
    bool outputFlug = true;
    std::string status = "OK";

    while (getCommitting() == 1) {
        if (i > 300) {
            return "error:timeout\n";
        }
        tSleep(10);
        i++;
    }
    setCommitting(1);
    if (inputs[0] == u8"stop") {
        stop();
    }
    else if (inputs[0] == u8"playlist") {
        //vlcPlayer::pauseToggle();
    }
    else if (inputs[0] == u8"loopToggle") {
        loopToggle();
    }
    else if (inputs[0] == u8"repeatToggle") {
        repeatToggle();
    }
    else if (inputs[0] == u8"randomToggle") {
        randomToggle();
    }
    else if (inputs[0] == u8"pauseToggle") {
        pauseToggle();
    }
    else if (inputs[0] == u8"previous") {
        previous();
    }
    else if (inputs[0] == u8"next") {
        next();
    }
    else if (inputs[0] == u8"playItemAtIndex") {
        if (!playItemAtIndex(stoi(inputs[1]))) {
            status = "error";
        }
    }
    else if (inputs[0] == u8"add" || (inputs[0] == u8"playOrAdd" && getFlug())) {
        std::vector<std::filesystem::path> paths;
        for (unsigned int i = 1;i < inputs.size();i++) {
            paths.push_back(std::filesystem::u8path(inputs[1].c_str()));
        }
        if (!addMediaInList(paths))
            status = u8"error:couldn't load any file.";
    }
    else if (inputs[0] == u8"now") {
        tSleep(1);
    }
    else if (inputs[0] == u8"extensionMode") {
        if (inputs.size() < 1 || inputs[1] == u8"0")
            extensionMode = 0;
        else if (inputs[1] == u8"1")
            extensionMode = 1;
        else status = "error:input value incorrect!";
        return status;
    }
    else if (inputs[0] == u8"information") {
        std::string output = status + u8"\n";
        output += playerDataStr() + u8"\n";
        for (unsigned int i = 0;i < mediaDataList.size();i++) {
            mediaData mediaData = mediaDataList[i];
            output += mediaDataStr(mediaData) + u8"\n";
        }
        return output;
    }
    else if (inputs[0] == u8"load") {
        setCommitting(0);
        return u8"OK";
    }
    else {
        int mode = 0;
        int addMode = 0;
        int addInput = 0;
        //sstpThread::setChangeByMe(1);
        if (inputs[0] == u8"play" || inputs[0] == u8"play2" || inputs[0] == u8"playOrAdd") {
            //path_str = inputs[1];
            if (inputs[0] == u8"play2") mode = 1;
            inputs.erase(inputs.begin(), inputs.begin() + 1);
        }
        bool ended = getState() == libvlc_state_t::libvlc_Ended && getNowPlayingMediaID() + 1 ==mediaDataList.size();
        if (getFlug() == 0 || mode == 0 || ended) {
            using namespace std::filesystem;
            std::vector<path> paths;
            bool playFlug = false;
            for (unsigned int i = 0;i < inputs.size();i++) {
                path path = u8path(inputs[i].c_str());

                if (loadPath(path, addMode))
                    playFlug = true;
                addMode = 1;
            }
            if (playFlug) {
                if (getFlug() != 0) {
                    stop();
                }

                sortMediaDataList();
                setMedia();
                play();
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
    if (playerState == NULL) {
        playerState = libvlc_state_t::libvlc_Error;
    }
    //printf("---OUTPUT----\n");
    output = status + u8"\n";
    if (vlcPlayer::getFlug()) {

        //output += std::to_string(playerState) + u8"\n";
        //bool x = playerState == libvlc_state_t::libvlc_Opening || playerState == libvlc_state_t::libvlc_Playing || playerState == libvlc_state_t::libvlc_Paused;
        mediaData mediaData = vlcPlayer::getNowPlaying();
        output += playerDataStr() + u8"\n";

        output += mediaDataStr(mediaData);
        //output += u8"\nstatus\n";
        //string_t lpname=;


    }
    else {
        output += u8"error:\n";
        output += errorMess;
    }
    vlcPlayer::setCommitting(0);
    return output;
}

void vlcPlayer::DispTitleList() {
    for (unsigned int i = 0;i < mediaDataList.size();i++) {
        mediaData mediaData = mediaDataList[i];
        printf("%d-%d:%s\n", mediaData.discNumber, mediaData.trackNumber, mediaData.title.c_str());
    }
}


/*void vlcPlayer::endHundler(const libvlc_event_t* event, void* param) {
    OnVaLCaTest();
}*/
void vlcPlayer::nextFunc() {
    libvlc_playback_mode_t mode = getPlaybackMode();
    mtxVLC_.lock();
    int x = played;
    played = 1;
    mtxVLC_.unlock();
    int mediaID = vlcPlayer::getNowPlayingMediaID();
    if (mode == libvlc_playback_mode_repeat) {
        OnVaLCaSSTP(u8"OnVaLCaRepeat");
    }
    else if (mediaID == 0) {
        if (x == 1)
            OnVaLCaLoop();

    }
    else {
        OnVaLCaPass();
    }
}
void vlcPlayer::nextHundler(const libvlc_event_t* event) {
    //int* size = (int*)param;
    std::thread nextThread(& vlcPlayer::nextFunc,this);
    nextThread.detach();
    return;
}
void vlcPlayer::playedHundler(const libvlc_event_t* event) {
    mtxVLC_.lock();
    played = 0;
    mtxVLC_.unlock();
    OnVaLCaFinish();
}

bool vlcPlayer::loadex() {

    //vlcPlayer vlc;
    //vlcPlayer::setCommitting(1);
    //vlcPlayer::initialize();
    //playerインスタンスの作成
    //vlcPlayer::setCommitting(0);


//    sstpThread::setState(1);
//    sstpThread::setChangeByMe(0);
    //std::thread testThread(testthread);
    //testThread.detach();

    //std::thread sstpThread(sstpthread);
    //sstpThread.detach();

    return true;
}
bool vlcPlayer::unloadex() {
    //    sstpThread::setState(0);
    tSleep(100);
    if (isPlaying())
        stop();
    release();

    return true;
}


//namespace VaLCaFunc {
    std::string mediaDataStr(mediaData mediaData) {
        std::string output = "";
        output += mediaData.title + u8"\n";
        output += mediaData.albumName + u8"\n";
        output += mediaData.artist + u8"\n";
        output += mediaData.path + u8"\n";
        output += std::to_string(mediaData.trackNumber) + u8"\n";
        output += std::to_string(mediaData.trackTotal) + u8"\n";
        output += std::to_string(mediaData.discNumber) + u8"\n";
        output += std::to_string(mediaData.discTotal) + u8"\n";
        output += timeStr(mediaData.length) + u8"\n";


        return output;
    }

    void tSleep(int x) {
        std::this_thread::sleep_for(std::chrono::microseconds(x * 1000));
    }
    std::string timeStr(libvlc_time_t time) {

        char timeStr[19] = "";
        sprintf_s(timeStr, 19, "%lld", time);
        return toStr(timeStr);
    }

    std::string myGetCurrentDirectry() {
        const int max_path = MY_MAX_PATH;
        char buf[max_path];
        GetCurrentDirectoryA(max_path, buf);
        string_t rq = SAORI_FUNC::MultiByteToUnicode(buf, CP_SJIS);
        std::string u8buf = SAORI_FUNC::UnicodeToMultiByte(rq, SAORI_FUNC::StringtoCodePage("utf-8"));
        std::string res = u8buf;
        return res;
    }
    std::vector< std::string > myMatch(std::string const& text, std::regex const& re)
    {
        std::vector< std::string > result;

        std::smatch m; // match_results

        std::regex_match(text, m, re);

        for (auto&& elem : m)
        {// elemはsub_match
            result.push_back(elem.str());
        }

        return result;
    }
    std::string getHwnd(std::string fmo) {
        //std::string mPath = GetModulePath();
        std::string mPath = myGetCurrentDirectry();
        std::smatch m;
        std::regex r(R"([^\n\.]+\.ghostpath[^\n]+\n)");
        std::regex r2(R"(([^\n\.]+)\.ghostpath[^\n](\S+)[^\n]*\n)");
        std::regex rhwnd(R"([^\n\.]+\.hwnd[^a-zA-Z]+\n)");
        std::regex rhwnd2(R"(([^\n\.]+)\.hwnd[^a-zA-Z0-9]+(\d+)[^\d\n]*\n)");
        std::string fmo_cpy = fmo;

        std::vector<std::vector<std::string>> gPathFmos;
        std::vector<std::vector<std::string>> gHwndFmos;
        while (std::regex_search(fmo_cpy, m, r)) {
            std::string path_fmo = m.str();
            std::vector<std::string> path_fmoArray = myMatch(path_fmo, r2);
            gPathFmos.push_back(path_fmoArray);
            fmo_cpy = m.suffix();
        }

        std::string gKey = "";
        std::string res = u8":\n";
        //std::regex rd = ;
        std::filesystem::path mPathp = std::filesystem::u8path(mPath);
        std::filesystem::path mPathG = mPathp.parent_path().parent_path();
        for (unsigned int i = 0;i < gPathFmos.size();i++) {
            std::filesystem::path gPathp = std::filesystem::u8path(gPathFmos[i][2]);
            if (std::filesystem::equivalent(mPathG, gPathp)) {
                gKey = gPathFmos[i][1];
            }
        }
        fmo_cpy = fmo;

        while (std::regex_search(fmo_cpy, m, rhwnd)) {
            std::string hwnd_fmo = m.str();
            std::vector<std::string> hwnd_fmoArray = myMatch(hwnd_fmo, rhwnd2);
            gHwndFmos.push_back(hwnd_fmoArray);
            fmo_cpy = m.suffix();
        }
        for (unsigned int i = 0;i < gHwndFmos.size();i++) {
            if (gHwndFmos[i][1].find(gKey) != std::string::npos) {
                return gHwndFmos[i][2];
            }
        }

        return "error";

    }
    bool send_sstp(const std::string& str, void* hwnd)
    {

        if (hwnd) {
            DWORD result;

            COPYDATASTRUCT c;
            c.dwData = 9801;
            c.cbData = str.size();
            c.lpData = const_cast<char*>(str.c_str());

            ::SendMessageTimeout(reinterpret_cast<HWND>(hwnd),
                WM_COPYDATA,
                reinterpret_cast<WPARAM>(hwnd),
                reinterpret_cast<LPARAM>(&c),
                SMTO_ABORTIFHUNG, 1000, &result);
            return true;
        }
        else {
            return false;
        }
    }
    void sstpNotify(std::string event, std::vector<std::string> references) {
        const int sharedMemSize = 1024 * 64;
        HANDLE hFM = OpenFileMappingA(FILE_MAP_READ, FALSE, "Sakura");
        BYTE* lpStr = new BYTE[sharedMemSize];
        lpStr = (BYTE*)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, sharedMemSize);
        long length;
        memcpy(&length, lpStr, 4);
        char* l = new char[sharedMemSize - 4];
        memcpy(l, lpStr + 4, length - 4);
        if (hFM != NULL && lpStr != NULL) {
            string_t rq = SAORI_FUNC::MultiByteToUnicode(l, CP_SJIS);
            std::string u8LpStr = SAORI_FUNC::UnicodeToMultiByte(rq, SAORI_FUNC::StringtoCodePage("utf-8"));
            std::string hwnd = getHwnd(u8LpStr);
            std::regex rInt(R"(\d+)");
            if (!std::regex_match(hwnd, rInt)) {
                UnmapViewOfFile(lpStr);
                CloseHandle(hFM);
                delete[]l;
                return;
            }
            char lenbuff[8];
            sprintf_s(lenbuff, "%d", length);
            std::string lenStr = (const char*)lenbuff;
            UnmapViewOfFile(lpStr);
            std::string sstpStr = u8"";
            sstpStr += u8"NOTIFY SSTP/1.0\n";
            sstpStr += u8"Sender: VaLCa\n";
            sstpStr += u8"Event: " + event + u8"\n";
            for (unsigned int i = 0;i < references.size();i++) {
                sstpStr += u8"Reference" + std::to_string(i) + u8": " + references[i] + u8"\n";
            }
            sstpStr += u8"Charset: UTF-8\n\n";
            long hwndl = stol(hwnd);
            HWND hwndp = (HWND)(LONG_PTR)hwndl;
            send_sstp(sstpStr, hwndp);
        }

        CloseHandle(hFM);
        delete[]l;

    }
    /*void testsstp(std::vector<std::string> ref) {
        tSleep(1000);
        sstpNotify("OnVaLCaTest", ref);
    }*/
    void testsstp(std::string ref) {
        std::vector<std::string>refs = { ref };
        tSleep(1000);
        sstpNotify("OnVaLCaTest", refs);
    }
    void OnVaLCaSSTP(std::string command) {
        std::vector<std::string> references;
        sstpNotify(command, references);
    }
    void OnVaLCaPass() {
        std::vector<std::string> references;
        sstpNotify(u8"OnVaLCaPass", references);
    }
    void OnVaLCaTest() {
        std::vector<std::string> references;
        sstpNotify(u8"OnVaLCaTest", references);
    }
    void OnVaLCaLoop() {
        std::vector<std::string> references;
        sstpNotify(u8"OnVaLCaLoop", references);
    }
    void OnVaLCaFinish() {
        std::vector<std::string> references;
        sstpNotify(u8"OnVaLCaFinish", references);
    }


    //static mediaData initializer = { "__NULL__","__NULL__" ,"__NULL__" ,"__NULL__",0,0,0,0 };





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


    /*//^^^^^^^^^^^-----------sstpThread
    class sstpThread {
    private:
        static int state;
        static int changeByMe;
    public:
        //static std::thread threadsstp;
        static void setState(int x);
        static int getState();
        static void setChangeByMe(int x);
        static int getChangeByMe();
    };
    int sstpThread::changeByMe = 0;
    int sstpThread::state = 1;
    void sstpThread::setState(int x) {


        mtx_.lock();
        state = x;
        mtx_.unlock();
    }
    int sstpThread::getState() {
        mtx_.lock();
        int x = state;
        mtx_.unlock();
        return x;
    }
    void sstpThread::setChangeByMe(int x) {
        //std::lock_guard<std::mutex> lock(mtx_);
        mtx_.lock();

        changeByMe = x;
        mtx_.unlock();
    }
    int sstpThread::getChangeByMe() {
        //std::lock_guard<std::mutex> lock(mtx_);
        mtx_.lock();
        int x = changeByMe;
        mtx_.unlock();
        return x;
    }
    */



    //using namespace std::filesystem;
    int idInStrings(std::string str, std::vector<std::string> list) {
        for (unsigned int i = 0;i < list.size();i++) {

            //printf(" %s== %s\n", list[i].c_str(), str.c_str());
            if (list[i] == str) {
                return i;
            }
        }
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







    /*
    //sstpThread
    void sstpthread() {
        int mediaID = -1;
        int mediaState = 0;
        while (sstpThread::getState()) {
            while (vlcPlayer::getCommitting() == 1) {
                tSleep(50);
            }
            int newMediaID=vlcPlayer::getNowPlayingMediaID();
            int newMediaState = vlcPlayer::getState();
            if (newMediaID != mediaID && mediaID >= 0 && !sstpThread::getChangeByMe()) {
                if (newMediaID > mediaID)
                    OnVaLCaPass();
                if (newMediaID < mediaID)
                    OnVaLCaLoop();
            }
            if (vlcPlayer::isEnded() && mediaState != newMediaState && !sstpThread::getChangeByMe()) {
                if(vlcPlayer::mediaDataList.size()==1&& vlcPlayer::getPlaybackMode() == libvlc_playback_mode_loop)
                    OnVaLCaLoop();
                else {
                    tSleep(50);
                    if (vlcPlayer::isEnded())
                        OnVaLCaFinish();
                }

            }

            mediaID = newMediaID;
            mediaState = newMediaState;
            tSleep(100);
            sstpThread::setChangeByMe(0);
            //sstptest();
        }
    }
    */

    /*
    //----------------testThread
    void testthread() {
        tSleep(10000);
        //tSleep(300);
        std::vector<std::string> inputs;
        //BYTE* lpStr = new BYTE[1024*64];
        //delete[]lpStr;
        //inputs.push_back(u8"play");
        //inputs.push_back(u8"sample.m4a");
        //inputs.push_back(u8"sample.m4a");
        //std::string op=execute(inputs);

        //ee
        std::vector<std::string> references;
        //references.push_back(op);
        //references.push_back(errorMess);
        sstpNotify(u8"OnVaLCaTest", references);

        while(sstpThread::getState())
            tSleep(100);
    }*/
    std::vector<std::string>loadExList() {
        namespace fs = std::filesystem;
        //fs::path path = u8"VaLCaConfig.txt";
        fs::path path = getDllPath();
        path.replace_filename(u8"VaLCaConfig.txt");
        testsstp(path.u8string());
        std::vector<std::string> exts;
        std::string line;
        std::regex reExt(R"(\.\w+)");
        std::regex reSpace(R"(\s)");
        std::regex reExtMode(R"(extension:)");
        //std::regex reVideo("R(video:)");
        int readMode = 0;
        if (is_regular_file(path)) {
            std::ifstream ifs(path);
            if (!ifs.fail()) {
                while (std::getline(ifs, line)) {
                    line = std::regex_replace(line, reSpace, "");
                    if (std::regex_match(line, reExtMode)) {
                        readMode = 1;
                    }
                    if ((readMode == 1 || readMode == 0) && std::regex_match(line, reExt)) {
                        exts.push_back(line);
                    }
                }
            }
        }
        return exts;
    }

    std::filesystem::path getDllPath() {
        char path[MY_MAX_PATH];
        HMODULE hm = NULL;

        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)&loadExList, &hm) == 0) {
            int ret = GetLastError();
            //fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
            // Return or however you want to handle an error.
            return "";
        }
        if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
            int ret = GetLastError();
            //fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
            // Return or however you want to handle an error.
            return "";
        }
        //string_t rq = SAORI_FUNC::MultiByteToUnicode(path, CP_SJIS);
        //std::string u8path = u8pathSAORI_FUNC::UnicodeToMultiByte(rq, SAORI_FUNC::StringtoCodePage("utf-8"));
        return path;
    }
//}