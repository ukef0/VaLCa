#include "pch.h"
#include <inttypes.h>
#include "csaori.h"
#include "csaori_base.h"
#define NUM_META 5



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
#include <random>
#include <regex>
//#undef OpenFileMapping 
#include <windows.h>
#include <mutex>
#define NUM_KEY 8
std::mutex mtx_;
std::mutex mtxVLC_;
std::mutex mtxVLCcommit_;
std::mutex mtxVLCflg_;
std::string errorMess=u8"";
int* mediaSize;
//std::mutex mtx_;

//#define OpenFileMappingW OpenFileMapping
/*struct testInput {
    std::vector<string_t> args;

};
struct testOutput {
    string_t result;
    SAORIRESULT result_code;
};*/
std::string getHwnd(std::string fmo);
bool send_sstp(const std::string& str, void* hwnd);
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
void tSleep(int x) {
    std::this_thread::sleep_for(std::chrono::microseconds(x*1000));
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
    static int randomMode;

    //static libvlc_media_t* media;
    //static libvlc_media_player_t* media_player;
    static int flug;
    
    static libvlc_media_list_t* mediaList;
    static libvlc_media_list_player_t* mediaListPlayer;
    static int committing;
    static libvlc_event_manager_t* eventManager;
    static libvlc_playback_mode_t playbackMode;
public:
    static int played;
    static void setCommitting(int x);
    static int getCommitting();
    
    static int playable;
    static std::vector<mediaData> mediaDataList;
    vlcPlayer();
    static int getFlug();
    static void setFlug(int x);
    static bool getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData);
    //static bool albumLoad(std::vector<std::string>& fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles);
    static bool albumLoad(std::vector<std::string>& fileNames, int addFlug);
    static void sortMediaDataList(int start, int end);
    static int64_t getTime();
    static void initialize();
    static void loopToggle();
    static void repeatToggle();
    static void setRandomMode(int x);
    static int getRandomMode();
    static void setPlaybackMode(libvlc_playback_mode_t x);
    static libvlc_playback_mode_t getPlaybackMode();
    static void randomToggle();
    static void setMedia();
    static void play();
    static bool isPlaying();
    static bool isEnded();
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
    static bool addMediaInList(std::vector<std::filesystem::path> paths);
};
libvlc_instance_t* vlcPlayer::instance;
int vlcPlayer::played;
int  vlcPlayer::playable;
int  vlcPlayer::committing;
std::vector<mediaData>  vlcPlayer::mediaDataList;
//libvlc_media_t* vlcPlayer::media;
//libvlc_media_player_t* vlcPlayer::media_player;
libvlc_media_list_player_t* vlcPlayer::mediaListPlayer;
int vlcPlayer::flug;
int vlcPlayer::randomMode;
libvlc_media_list_t* vlcPlayer::mediaList;
libvlc_playback_mode_t vlcPlayer::playbackMode;
libvlc_event_manager_t* vlcPlayer::eventManager;
void endHundler(const libvlc_event_t* event, void* param);
void playedHundler(const libvlc_event_t* event, void* param);
void nextHundler(const libvlc_event_t* event, void* param);

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
    int x=state;
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
    int x=changeByMe;
    mtx_.unlock();
    return x;
}

bool vlcPlayer::isEnded() {
    mtxVLC_.lock();
    unsigned int x=vlcPlayer::mediaDataList.size();
    mtxVLC_.unlock();
    bool cond1= libvlc_state_t::libvlc_Ended == getState();
    bool cond2 = getNowPlayingMediaID()==x-1;
    return cond1 && cond2;
}
void vlcPlayer::setCommitting(int x) {
    mtxVLCcommit_.lock();
    committing = x;
    mtxVLCcommit_.unlock();
}
int vlcPlayer::getCommitting() {
    mtxVLCcommit_.lock();
    int x=committing;
    mtxVLCcommit_.unlock();
    return x;
}

void vlcPlayer::initialize() {
    mtxVLC_.lock();
    played = 0;
    instance = libvlc_new(0, NULL);
    mediaList = libvlc_media_list_new(instance);
    mediaListPlayer = libvlc_media_list_player_new(instance);
    mtxVLC_.unlock();
    setFlug(0);
    setRandomMode(0);
    setPlaybackMode(libvlc_playback_mode_default);
    mtxVLC_.lock();
    eventManager=libvlc_media_list_player_event_manager(mediaListPlayer);
    //eventManager2 = libvlc_media_list_player_event_manager(mediaListPlayer);
    //libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, endHundler,NULL);
    libvlc_event_attach(eventManager, libvlc_MediaListPlayerPlayed, playedHundler, mediaSize);
    libvlc_event_attach(eventManager, libvlc_MediaListPlayerNextItemSet, nextHundler, mediaSize);
    mtxVLC_.unlock();
    
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
    int x=randomMode;
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
    libvlc_state_t x=libvlc_media_list_player_get_state(mediaListPlayer);
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
    int x=libvlc_media_list_index_of_item(mediaList, mediaD);
    mtxVLC_.unlock();
    return x;
}
mediaData vlcPlayer::getNowPlaying() {
    int mediaID = getNowPlayingMediaID();
    //printf("NowPlaying mediaID:%d", mediaID);
    mtxVLC_.lock();
    mediaData x=mediaDataList[mediaID];
    mtxVLC_.unlock();
    return x;
}

void vlcPlayer::play() {
    mtxVLC_.lock();
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
    int flug=getFlug();
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
    int64_t x=libvlc_media_player_get_time(mediaPlayerD);
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
void vlcPlayer::release() {
    mtxVLC_.lock();
    //libvlc_event_detach(eventManager, libvlc_MediaPlayerEndReached, endHundler, NULL);
    libvlc_event_detach(eventManager, libvlc_MediaListPlayerPlayed, playedHundler, NULL);
    libvlc_event_detach(eventManager, libvlc_MediaListPlayerNextItemSet, nextHundler, NULL);
    libvlc_media_list_player_release(mediaListPlayer);
    libvlc_media_list_release(mediaList);
    libvlc_release(instance);
    mtxVLC_.unlock();
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
    sstpThread::setChangeByMe(1);
    mtxVLC_.lock();
    int x =libvlc_media_list_player_next(mediaListPlayer);
    mtxVLC_.unlock();
    return  x== 0;
}
bool vlcPlayer::previous() {
    sstpThread::setChangeByMe(1);
    mtxVLC_.lock();
    int x = libvlc_media_list_player_previous(mediaListPlayer);
    mtxVLC_.unlock();
    return x == 0;
}

bool vlcPlayer::getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData) {
    //mediaData mediaData;
    //mtxVLC_.lock();
    libvlc_media_t* mediaD = libvlc_media_new_path(instance, path.u8string().c_str());
    if (libvlc_media_parse_with_options(mediaD, libvlc_media_fetch_local, 1000) == 0) {
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
        //mtxVLC_.unlock();
        return true;
    }
    else {
        //mtxVLC_.unlock();
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
    //mtxVLC_.lock();
    for (unsigned int i = 0;i < fileNames.size();i++) {
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
    //mtxVLC_.unlock();
    tSleep(120);
    unsigned int count = 0;
    if (mediaV.size() < 1) {
       
        return false;
    }
        
    while (libvlc_media_get_meta(mediaV[mediaV.size() - 1], libvlc_meta_TrackNumber) == NULL||
        libvlc_media_get_duration(mediaV[mediaV.size() - 1])==NULL|| libvlc_media_get_duration(mediaV[mediaV.size() - 1]) <= 1) {
        //tSleep(120 + 100 * fileNames.size());
        tSleep(120);
        count++;
        if (count > fileNames.size() * 6) {
            break;
        }
    }
    tSleep(50);


    //std::vector <mediaData> meta(mediaV.size());
    //std::vector <std::string> albumNames;
    //mtxVLC_.lock();
    int n = -1;
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
    *mediaSize = mediaDataList.size();
    //sortMediaDataList();
    //mtxVLC_.unlock();
    return true;
    
}
bool loadPath(std::filesystem::path path, int addFlug = 0) {
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
        if (vlcPlayer::albumLoad(fileNames, addFlug)) {
            playFlug = true;
        }
        else {
            errorMess += u8"vlc couldn't load directory\nmay be there are n't files?\n";
            return false;
        }
    }
    else {
        mediaData mediaData;
        if (!vlcPlayer::getMediaDataWithPath(path, mediaData)) {
            errorMess += u8"vlc couldn't load file\nmay be there are n't files?\n";
            return false;
        }
        else {
           // mtxVLC_.lock();
            if (addFlug == 0) {
                vlcPlayer::mediaDataList.clear();
            }
            vlcPlayer::mediaDataList.push_back(mediaData);
            *mediaSize= vlcPlayer::mediaDataList.size();
            //mtxVLC_.unlock();
        }
    }

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
    if (res==1) {
        
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
    *mediaSize = mediaDataList.size();
    mtxVLC_.unlock();
}
void vlcPlayer::getPathList(std::vector<std::string>& pathList) {
    mtxVLC_.lock();
    for (unsigned int i = 0;i < mediaDataList.size();i++) {
        pathList.push_back(mediaDataList[i].path);
    }
    mtxVLC_.unlock();
}


void DispTitleList() {
    for (unsigned int i = 0;i < vlcPlayer::mediaDataList.size();i++) {
        mediaData mediaData = vlcPlayer::mediaDataList[i];
        printf("%d-%d:%s\n", mediaData.discNumber, mediaData.trackNumber, mediaData.title.c_str());
    }
}



std::string execute(std::vector<std::string>inputs) {
    //std::lock_guard<std::mutex> lock(mtx_);
    int i = 0;
    bool outputFlug = true;
    std::string status = "OK";
    
    while (vlcPlayer::getCommitting() == 1) {
        if (i > 300) {
            return "error:timeout\n";
        }
        tSleep(10);
        i++;
    }
    vlcPlayer::setCommitting(1);
    if (inputs[0] == u8"stop") {
        vlcPlayer::stop();
    }
    else if (inputs[0] == u8"playlist") {
        //vlcPlayer::pauseToggle();
    }
    else if (inputs[0] == u8"loopToggle") {
        vlcPlayer::loopToggle();
    }
    else if (inputs[0] == u8"repeatToggle") {
        vlcPlayer::repeatToggle();
    }
    else if (inputs[0] == u8"randomToggle") {
        vlcPlayer::randomToggle();
    }
    else if (inputs[0] == u8"pauseToggle") {
        vlcPlayer::pauseToggle();
    }
    else if (inputs[0] == u8"previous") {
        vlcPlayer::previous();
        tSleep(80);
    }
    else if (inputs[0] == u8"next") {
        vlcPlayer::next();
        tSleep(80);
    }
    else if (inputs[0] == u8"add" || (inputs[0] == u8"playOrAdd" && vlcPlayer::getFlug())) {
        std::vector<std::filesystem::path> paths;
        for (unsigned int i = 1;i < inputs.size();i++) {
            paths.push_back(std::filesystem::u8path(inputs[1].c_str()));
        }
        vlcPlayer::addMediaInList(paths);
        tSleep(80);
    }
    else if (inputs[0] == u8"now") {
        tSleep(80);
    }
    else if (inputs[0] == u8"load") {
        vlcPlayer::setCommitting(0);
        return u8"OK";
    }
    else {
        int mode = 0;
        int addMode = 0;
        int addInput = 0;
        sstpThread::setChangeByMe(1);
        if (inputs[0] == u8"play" || inputs[0] == u8"play2" || inputs[0] == u8"playOrAdd") {
            //path_str = inputs[1];
            if (inputs[0] == u8"play2") mode = 1;
            inputs.erase(inputs.begin(), inputs.begin() + 1);
        }
        bool ended = vlcPlayer::getState() == libvlc_state_t::libvlc_Ended && vlcPlayer::getNowPlayingMediaID() + 1 == vlcPlayer::mediaDataList.size();
        if (vlcPlayer::getFlug() == 0 || mode == 0 || ended) {
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
                if (vlcPlayer::getFlug != 0) {
                    vlcPlayer::stop();
                }

                vlcPlayer::sortMediaDataList();
                vlcPlayer::setMedia();
                vlcPlayer::play();
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
    if (vlcPlayer::getFlug()){
        
        output += std::to_string(playerState) + u8"\n";
        bool x = playerState == libvlc_state_t::libvlc_Opening || playerState == libvlc_state_t::libvlc_Playing || playerState == libvlc_state_t::libvlc_Paused;
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
        output += std::to_string(vlcPlayer::getPlaybackMode()) + u8"\n";
        output += u8"\nstatus\n";
        //string_t lpname=;
        
        
    }else {
        output += u8"error:\n";
        output += errorMess;
    }
    vlcPlayer::setCommitting(0);
    return output;
}

std::string myGetCurrentDirectry() {
    const int max_path = 1000;
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
        std::string path_fmo=m.str();
        std::vector<std::string> path_fmoArray = myMatch(path_fmo, r2);
        gPathFmos.push_back(path_fmoArray);
        fmo_cpy = m.suffix();
    }

    std::string gKey="";
    std::string res = u8":\n";
    //std::regex rd = ;
    std::filesystem::path mPathp = std::filesystem::u8path(mPath);
    std::filesystem::path mPathG = mPathp.parent_path().parent_path();
    for (unsigned int i = 0;i < gPathFmos.size();i++) {
        std::filesystem::path gPathp = std::filesystem::u8path(gPathFmos[i][2]);
        if(std::filesystem::equivalent(mPathG,gPathp)){
            gKey= gPathFmos[i][1];
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
void sstpNotify(std::string event,std::vector<std::string> references) {
    const int sharedMemSize = 1024 *64;
    HANDLE hFM = OpenFileMappingA(FILE_MAP_READ, FALSE, "Sakura");
    BYTE* lpStr =new BYTE[sharedMemSize];
    lpStr = (BYTE*)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, sharedMemSize);
    long length;
    memcpy(&length, lpStr, 4);
    char* l = new char[sharedMemSize - 4];
    memcpy(l, lpStr + 4, length - 4);
    if (hFM != NULL && lpStr!= NULL) {
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
        sstpStr += u8"Event: "+event+u8"\n";
        for (unsigned int i = 0;i < references.size();i++) {
            sstpStr += u8"Reference"+std::to_string(i)+u8": "+ references[i]+u8"\n";
        }
        sstpStr += u8"Charset: UTF-8\n\n";
        long hwndl = stol(hwnd);
        HWND hwndp = (HWND)(LONG_PTR)hwndl;
        send_sstp(sstpStr, hwndp);
    }

    CloseHandle(hFM);
    delete[]l;

}

void OnVaLCaSSTP(std::string command) {
    std::vector<std::string> references;
    sstpNotify(command, references);
}
void OnVaLCaPass() {
    std::vector<std::string> references;
    sstpNotify(u8"OnVaLCaPass",references);
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
void endHundler(const libvlc_event_t* event, void* param) {
    OnVaLCaTest();
}
void nextFunc() {
    libvlc_playback_mode_t mode = vlcPlayer::getPlaybackMode();
    mtxVLC_.lock();
    int x = vlcPlayer::played;
    vlcPlayer::played = 1;
    mtxVLC_.unlock();
    int mediaID = vlcPlayer::getNowPlayingMediaID();
    if (mode == libvlc_playback_mode_repeat) {
        OnVaLCaSSTP(u8"OnVaLCaRepeat");
    }
    else if (mediaID == 0) {
        if (x == 1)
            OnVaLCaLoop();
        
    }else {
        OnVaLCaPass();
    }
}
void nextHundler(const libvlc_event_t* event, void* param) {
    int* size=(int*)param;
    std::thread nextThread(nextFunc);
    nextThread.detach();
    return;
}
void playedHundler(const libvlc_event_t * event, void* param) {
    mtxVLC_.lock();
    vlcPlayer::played = 0;
    mtxVLC_.unlock();
    OnVaLCaFinish();
}
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
}
bool loadex() {
    int* x=new int;
    *x = 0;
    mediaSize = x;
    
    //vlcPlayer::setCommitting(1);
    //playerインスタンスの作成
    vlcPlayer::setCommitting(0);
    vlcPlayer::initialize();
    
    sstpThread::setState(1);
    sstpThread::setChangeByMe(0);
    //std::thread testThread(testthread);
    //testThread.detach();

    //std::thread sstpThread(sstpthread);
    //sstpThread.detach();
    
    return true;
}
bool unloadex() {
    sstpThread::setState(0);
    tSleep(100);
    if (vlcPlayer::isPlaying())
        vlcPlayer::stop();
    vlcPlayer::release();
    delete mediaSize;
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
        std::string arg=SAORI_FUNC::UnicodeToMultiByte(in.args[i], SAORI_FUNC::StringtoCodePage("utf-8"));
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
