//このファイルは使わないのでプロジェクトから除外すること

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
#include <random>
#define NUM_KEY 8
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
static mediaData initializer = {"__NULL__","__NULL__" ,"__NULL__" ,"__NULL__",0,0,0,0};

bool operator <(const mediaData left, const mediaData right) {
    int compare = left.albumName.compare(right.albumName);
    if (compare==0) {
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
    static int randomMode;
    
    //static libvlc_media_t* media;
    //static libvlc_media_player_t* media_player;
    static int flug;
    static libvlc_media_list_t* mediaList;
    static libvlc_media_list_player_t* mediaListPlayer;
public:
    static int committing;
    static libvlc_playback_mode_t playbackMode;
    static int playable;
    static std::vector<mediaData> mediaDataList;
    vlcPlayer();
    static int getFlug();
    static bool getMediaDataWithPath(std::filesystem::path path,mediaData& mediaData);
    //static bool albumLoad(std::vector<std::string>& fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles);
    static bool albumLoad(std::vector<std::string>& fileNames,int addFlug);
    static void sortMediaDataList(int start, int end);
    static int64_t getTime();
    static void initialize();
    static void loopToggle();
    static void repeatToggle();
    static void setRandomMode(int x);
    static void randomToggle();
    static void setMedia();
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
    static bool addMediaInList(std::filesystem::path path);
};
libvlc_instance_t* vlcPlayer::instance;
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


void vlcPlayer::initialize() {
    instance = libvlc_new(0, NULL);
    mediaList = libvlc_media_list_new(instance);
    mediaListPlayer=libvlc_media_list_player_new(instance);
    flug = 0;
    randomMode = 0;
    playbackMode= libvlc_playback_mode_default;
}
void vlcPlayer::loopToggle() {
    printf("looptoggle\n");
    if (playbackMode != libvlc_playback_mode_loop)
        playbackMode = libvlc_playback_mode_loop;
    else
        playbackMode = libvlc_playback_mode_default;
    libvlc_media_list_player_set_playback_mode(mediaListPlayer, playbackMode);
}
void vlcPlayer::repeatToggle() {
    printf("repeattoggle\n");
    if (playbackMode != libvlc_playback_mode_repeat)
        playbackMode = libvlc_playback_mode_repeat;
    else
        playbackMode = libvlc_playback_mode_default;
    libvlc_media_list_player_set_playback_mode(mediaListPlayer, playbackMode);
}
void vlcPlayer::setRandomMode(int x) {
    randomMode = x;
}
void vlcPlayer::randomToggle() {
    randomMode = 1-randomMode;
}
libvlc_state_t vlcPlayer::getState() {
    return libvlc_media_list_player_get_state(mediaListPlayer);
}
void DispStr(const char* name,const char* str) {
    if (str == NULL)
        printf("%s: NULL\n",name);
    else
        printf("%s: %s\n",name, str);
}
void vlcPlayer::setMedia() {
    libvlc_media_list_release(mediaList);
    mediaList = libvlc_media_list_new(instance);
    for (int i = 0;i < mediaDataList.size();i++) {
        libvlc_media_t* mediaD= libvlc_media_new_path(instance, mediaDataList[i].path.c_str());
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
    int mediaID=getNowPlayingMediaID();
    //printf("NowPlaying mediaID:%d", mediaID);
    return mediaDataList[mediaID];
}

void vlcPlayer::play() {
    libvlc_media_list_player_play(mediaListPlayer);
    Sleep(100);
    libvlc_media_player_t* mediaPlayerD = libvlc_media_list_player_get_media_player(mediaListPlayer);
    libvlc_media_t* mediaD=libvlc_media_player_get_media(mediaPlayerD);
    const char* nowplaying = libvlc_media_get_meta(mediaD, libvlc_meta_Title);
    //DispStr("nowplaying", nowplaying);
}

void vlcPlayer::pauseToggle() {
    libvlc_state_t state=getState();
    printf("state:%d\n",state);
    if (getFlug() && libvlc_state_t::libvlc_Playing == state) {
        
        libvlc_media_list_player_pause(mediaListPlayer);
    }
    else {
        vlcPlayer::play();
    }
}
int64_t vlcPlayer::getTime(){
    libvlc_media_player_t* mediaPlayerD=libvlc_media_list_player_get_media_player(mediaListPlayer);
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
        
        printf(" %s== %s\n",list[i].c_str(), str.c_str());
        if (list[i] == str) {
            return i;
        }
    }
    return -1;
}
bool filesInFolder(const char* input_path, std::vector<std::string>& fileNames, int mode){
    using namespace std::filesystem;
    path path = u8path(input_path);
    directory_iterator iter(path), end;
    //std::vector<std::string> fileNames;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const directory_entry entry = *iter;
        //path entryPath=entry.path();
        std::string pathChar=entry.path().filename().u8string();
        if (pathChar[0] != u8'.'){
            if (is_directory(entry.path())) {
                if(mode==1) filesInFolder(entry.path().u8string().c_str(), fileNames,mode);
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
    return libvlc_media_list_player_next(mediaListPlayer)==0;
}
bool vlcPlayer::previous() {
    return libvlc_media_list_player_previous(mediaListPlayer)==0;
}

bool vlcPlayer::getMediaDataWithPath(std::filesystem::path path,mediaData& mediaData) {
    //mediaData mediaData;
    libvlc_media_t* mediaD = libvlc_media_new_path(instance,path.u8string().c_str());
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
bool vlcPlayer::albumLoad(std::vector<std::string> &fileNames,int addFlug=0) {//----------第二

    //printf("===albumLoad===\n");
    std::vector <libvlc_media_t*> mediaV;
    int k = 0;
    if (fileNames.size() < 1) {
        return false;
    }
    for (unsigned int i = 0;i < fileNames.size();i++) {
        libvlc_media_t* media=libvlc_media_new_path(instance, fileNames[i].c_str());
        mediaV.push_back(media);
        
        if (libvlc_media_parse_with_options(mediaV.back(), libvlc_media_fetch_local,1000) != 0) {
            fileNames.erase(fileNames.begin() + i);
            mediaV.pop_back();
            libvlc_media_release(media);
            i--;
            //printf("couldn't be opened\n");
        }
        //printf("load%d\n", i);
    }
    Sleep(120);
    unsigned int count = 0;
    if (mediaV.size() < 1) return false;
    while (libvlc_media_get_meta(mediaV[mediaV.size() - 1], libvlc_meta_TrackNumber) == NULL) {
        //Sleep(120 + 100 * fileNames.size());
        Sleep(120);
        count++;
        if (count > fileNames.size() * 2) {
            break;
        }
    }
    Sleep(50);
    
    
    //std::vector <mediaData> meta(mediaV.size());
    //std::vector <std::string> albumNames;
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
    //sortMediaDataList();
    return true;
}
bool loadPath(std::filesystem::path path, int addFlug = 0) {
    bool playFlug = false;
    std::vector<std::string> pathList;
    if (is_directory(path)) {
        std::vector <std::string> fileNames;
        //std::map<std::string, std::vector<mediaData>> albumMap;
        filesInFolder(path.u8string().c_str(), fileNames, 1);
        //if (vlcPlayer::albumLoad(fileNames, albumMap)) {
        if (vlcPlayer::albumLoad(fileNames, addFlug)) {
            playFlug = true;
        }
        else {
            return false;
        }
    }
    else {
        mediaData mediaData;
        if (!vlcPlayer::getMediaDataWithPath(path, mediaData)) {
            return false;
        }
        else {
            if (addFlug == 0) {
                vlcPlayer::mediaDataList.clear();
            }
            vlcPlayer::mediaDataList.push_back(mediaData);

        }
    }

    return true;
}
bool vlcPlayer::addMediaInList(std::filesystem::path path) {
    int mediaDataSize=mediaDataList.size();
    printf("loadsize:%d\n", mediaDataSize);
    if (loadPath(path,1)) {
        sortMediaDataList(mediaDataSize,0);
        printf("loadsize:%d\n", mediaDataList.size());
        for (int i = mediaDataSize;i < mediaDataList.size();i++) {
            libvlc_media_t* mediaD = libvlc_media_new_path(instance, mediaDataList[i].path.c_str());
            libvlc_media_list_add_media(mediaList, mediaD);
            libvlc_media_release(mediaD);
        }
        return true;
    }
    else {
        return false;
    }

}
void vlcPlayer::sortMediaDataList(int start=0, int end=0) {

    if (randomMode == 1) {
        std::mt19937 get_rand_mt((unsigned int)time(NULL));
        std::shuffle(mediaDataList.begin()+start, mediaDataList.end()-end, get_rand_mt);
    }
    else {
        std::sort(mediaDataList.begin()+start, mediaDataList.end()-end);
        //printf("===normalsorting===\n");
    }
}
void vlcPlayer::setMediaDataList( std::map<std::string, std::vector<mediaData>>& albumFiles) {
    for (auto itr = albumFiles.begin(); itr != albumFiles.end(); ++itr) {
        for (unsigned int i = 0;i < itr->second.size();i++) {
            mediaDataList.push_back(itr->second[i]);
        }
    }
}
void vlcPlayer::getPathList( std::vector<std::string>& pathList) {
    for (unsigned int i = 0;i <mediaDataList.size();i++) {
            pathList.push_back(mediaDataList[i].path);
    }
}


bool load() {
    vlcPlayer::committing = 1;
    //playerインスタンスの作成
    vlcPlayer::initialize();
    vlcPlayer::committing = 0;
    return true;
}
bool unload() {
    if(vlcPlayer::isPlaying())
        vlcPlayer::stop();
    vlcPlayer::release();
    return true;
}
void DispTitleList() {
    for (unsigned int i = 0;i < vlcPlayer::mediaDataList.size();i++) {
        mediaData mediaData=vlcPlayer::mediaDataList[i];
        printf("%d-%d:%s\n" ,mediaData.discNumber, mediaData.trackNumber,mediaData.title.c_str());
    }
}
    

std::string exec(std::vector<std::string>inputs) {
    clock_t start = clock();
    //printf("====EXEC====");
    int i=0;
    bool outputFlug = true;
    std::string status="OK";

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
        Sleep(80);
    }
    else if (inputs[0] == u8"next") {
        vlcPlayer::next();
        Sleep(80);
    }
    else if (inputs[0] == u8"add"||(inputs[0] == u8"playOrAdd" && vlcPlayer::getFlug())) {
        
        std::filesystem::path path = std::filesystem::u8path(inputs[1].c_str());
        vlcPlayer::addMediaInList(path);
        Sleep(80);
    }
    else if (inputs[0] == u8"now") {
        Sleep(80);
    }
    else {
        //std::vector<std::string> path_strs;
        int mode = 0;
        int addMode = 0;
        int addInput = 0;
        
        if (inputs[0] == u8"play"||inputs[0] == u8"play2"|| inputs[0] == u8"playOrAdd") {
            //path_str = inputs[1];
            if (inputs[0] == u8"play2") mode = 1;
            inputs.erase(inputs.begin(), inputs.begin() + 1);

        }
        bool ended=vlcPlayer::getState()==libvlc_Ended&& vlcPlayer::getNowPlayingMediaID()+1== vlcPlayer::mediaDataList.size();
        if (vlcPlayer::getFlug()==0||mode == 0||ended) {
            using namespace std::filesystem;
            std::vector<path> paths;
            bool playFlug = false;
            for (unsigned int i = 0;i < inputs.size();i++) {
                path path =u8path(inputs[i].c_str());
                
                if (loadPath(path,addMode))
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
                printf("====DISPTITLELIST====\n");
                DispTitleList();
                printf("^^^^^DISPTITLELIST^^^^\n");
            }
            else {
                printf("====EXEC===error\nd");
                status = "error:couldn't load the file(s)";
                outputFlug = false;
            }

        }
        else {
            status = "error:playing now!";
        }

    }
    std::string output;
    libvlc_state_t playerState=vlcPlayer::getState();
    //char buffer[50]; 
    if (playerState == NULL) {
        playerState = libvlc_Error;
        //printf("playerState==NULL\n");
    }
    //printf("---OUTPUT----\n");
    output = status + u8"\n";
    output += std::to_string(playerState)+u8"\n";
    bool x = playerState == libvlc_Opening || playerState == libvlc_Playing || playerState == libvlc_Paused;
    mediaData mediaData=vlcPlayer::getNowPlaying();
    output += mediaData.title + u8"\n";
    output += mediaData.albumName + u8"\n";
    output += mediaData.artist + u8"\n";
    output += mediaData.path + u8"\n";
    output += std::to_string(mediaData.trackNumber)+ u8"\n";
    output += std::to_string(mediaData.trackTotal) + u8"\n";
    output += std::to_string(mediaData.discNumber) + u8"\n";
    output += std::to_string(mediaData.discTotal) + u8"\n";
    output += timeStr(mediaData.length) + u8"\n";
    output += timeStr(vlcPlayer::getTime()) + u8"\n";
    output += std::to_string(vlcPlayer::getNowPlayingMediaID()) + u8"\n";
    output += std::to_string(vlcPlayer::mediaDataList.size()) + u8"\n";
    output += std::to_string(vlcPlayer::playbackMode) + u8"\n";
    printf("%s", output.c_str());
    clock_t end = clock();
    const double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
    //printf("time %lf[ms]\n", time);
    vlcPlayer::committing = 0;
    return output;
}

void keyVerify(bool* key) {
    key[0] = (GetKeyState(VK_SPACE) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[1] = (GetKeyState(VK_LEFT) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[2] = (GetKeyState(VK_RIGHT) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[3] = (GetKeyState(0x31) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[4] = (GetKeyState(0x32) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[5] = (GetKeyState(0x33) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[6] = (GetKeyState(0x34) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
    key[7] = (GetKeyState(0x35) & 0x8000) && (GetKeyState(VK_LSHIFT) & 0x8000);
}
void keyCopy(bool* key, bool* key2) {
    for (int i = 0;i < NUM_KEY;i++) {
        key2[0] = key[0];
    }
}
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    load();
    unload();
    printf("hello\n");
    if (1) {
        // char input_path[200];
        //std::string input_path = u8"C:\\Users\\shono.f\\Desktop\\Jungle Science  9 Lazy 9 - Paradise Blown (Ninja Tune ZENCD009, 1994) (5).mp3";
        //std::string input_path = u8"F:\\QuickTime Player\\SR320";
        //std::string input_path=u8"C:\\Users\\shono.f\\Desktop\\Hélène De Montgeroult";
        //std::string input_path=u8"C:\\Users\\shono.f\\Desktop\\frenchsuite";
        //std::string input_path = u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Glenn Gould\\J.S.Bach_ Well-Tempered Clavier, Book 1, BWV 854-861";
        //std::string input_path=u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Glenn Gould\\Bach_ Well-Tempered Clavier, Book 2, BWV 886-893";
        std::string input_path = u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Edna Stern\\Hélène De Montgeroult";
        //std::string input_path2 = u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\9 Lazy 9";
        std::string input_path2 = u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Fanfare Ciocarlia";
        load();
        std::vector<std::string>inputs;
        inputs.push_back(input_path);
        std::string output=exec(inputs);
        //printf("%s", output.c_str());
        
        //vlcPlayer Player2(u8"C:\\Users\\shono.f\\Desktop\\SRパレット.m4a");
        //vlcPlayer::play();
        /* 待機する */

        //while (vlcPlayer::isPlaying()){
        
        bool key[NUM_KEY];
        std::vector<std::string> keyCommand(NUM_KEY); 
        keyCommand[0] = u8"pauseToggle";
        keyCommand[1] = u8"previous";
        keyCommand[2] = u8"next";
        //keyCommand[3] = u8"F:\\QuickTime Player\\SR320";
        keyCommand[3] = input_path2;
        keyCommand[4] = u8"repeatToggle";
        keyCommand[5] = u8"loopToggle";
        keyCommand[6] = u8"randomToggle";
        keyCommand[7] = u8"playOrAdd";
        keyVerify(key);
        bool newKey[NUM_KEY];
        keyCopy(key, newKey);
        std::vector<std::string> in;
        while (1) {
            Sleep(50);
            keyVerify(newKey);
            for (int i = 0;i < NUM_KEY;i++) {
                if (newKey[i] && newKey[i] != key[i]) {
                    in.clear();

                    in.push_back(keyCommand[i]);
                    if (i == 7) {
                        in.push_back(u8"F:\\shono\\Music\\iTunes\\iTunes Media\\Music\\Fanfare Ciocarlia");
                    }
                    clock_t start = clock();
                    exec(in);
                    clock_t end = clock();
                    const double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
                    //printf("time %lf[ms]\n", time);
                    continue;
                }
            }
            
            keyCopy(newKey,key);

        }
        unload();
        /* 再生を止める */
    }

    
    return 0;
}
