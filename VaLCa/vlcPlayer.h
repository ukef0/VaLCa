#pragma once
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>
#include <filesystem>
#define MY_MAX_PATH 1000
//#include "VaLCa_util.h"
#ifndef ___VALCA___
#define ___VALCA___
//struct mediaData;
namespace {
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
}
class vlcPlayer {
private:
    std::mutex mtxVLC_;
    std::mutex mtx_;
    std::mutex mtxVLCcommit_;
    std::mutex mtxVLCflg_;
    libvlc_instance_t* instance;
    int randomMode;//------------1だと、再生リスト作成時に、ランダムソートをする。追加曲は追加部分のみランダムソート。
    int flug;//------------------初期に0で、初回の楽曲再生で1になる。

    libvlc_media_list_t* mediaList;//------vlc本体のプレイリスト
    libvlc_media_list_player_t* mediaListPlayer;
    int committing;//------------execによる処理が行われている間1、行動の重複を防ぐ。
    libvlc_event_manager_t* eventManager;
    libvlc_playback_mode_t playbackMode;//---------ループ再生、リピート再生か、デフォルト再生か
    std::string errorMess;
    long gHwnd;

public:
    static void CallbackNextFunc(const libvlc_event_t* event,void* arg);
    static void CallbackPlayedFunc(const libvlc_event_t* event,void* arg);
    vlcPlayer();
    int extensionMode;//--------------1なら拡張子を監視して再生するか区別。0だと全部再生しようとする。
    std::vector<std::string> extensionList;//-------対応拡張子のリスト
    int played;//---------------1で最初の曲を再生するとOnLoopイベント。再生ボタンによる再生で0にされる。
    std::vector<mediaData> mediaDataList;//-----------プレイリスト中の曲データの配列
    

    void setCommitting(int x);
    int getCommitting();

    int getFlug();
    void setFlug(int x);

    void initialize();
    void release();

    void loopToggle();
    void repeatToggle();

    void setRandomMode(int x);
    int getRandomMode();
    void randomToggle();
    void setPlaybackMode(libvlc_playback_mode_t x);
    libvlc_playback_mode_t getPlaybackMode();
    void setHwnd(long x);
    long getHwnd();

    void setMedia();
    bool getMediaDataWithPath(std::filesystem::path path, mediaData& mediaData);
    //bool albumLoad(std::vector<std::string>& fileNames, std::map<std::string, std::vector<mediaData>>& albumFiles);
    bool albumLoad(std::vector<std::string>& fileNames, int addFlug);
    void setMediaDataList(std::map<std::string, std::vector<mediaData>>& albumFiles);
    void sortMediaDataList(int start, int end);
    void getPathList(std::vector<std::string>& pathList);

    bool isPlaying();
    bool isEnded();

    bool setVolume(int x);
    int getVolume();


    void play();
    void stop();
    void pauseToggle();
    bool next();
    bool previous();
    bool playItemAtIndex(int id);

    int64_t getTime();
    libvlc_state_t getState();
    std::string vlcPlayer::playerDataStr();

    mediaData getNowPlaying();
    int getNowPlayingMediaID();
    bool addMediaInList(std::vector<std::filesystem::path> paths);
    std::string vlcPlayer::execute(std::vector<std::string>inputs);
    void DispTitleList();

    //void endHundler(const libvlc_event_t* event, void* param);
    void nextFunc();
    void vlcPlayer::nextHundler(const libvlc_event_t* event);
    void vlcPlayer::playedHundler(const libvlc_event_t* event);
    bool vlcPlayer::loadPath(std::filesystem::path path, int addFlug);
    bool vlcPlayer::loadex();
    bool vlcPlayer::unloadex();
};
#endif