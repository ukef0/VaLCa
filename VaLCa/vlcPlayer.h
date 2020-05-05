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
    int randomMode;//------------1���ƁA�Đ����X�g�쐬���ɁA�����_���\�[�g������B�ǉ��Ȃ͒ǉ������̂݃����_���\�[�g�B
    int flug;//------------------������0�ŁA����̊y�ȍĐ���1�ɂȂ�B

    libvlc_media_list_t* mediaList;//------vlc�{�̂̃v���C���X�g
    libvlc_media_list_player_t* mediaListPlayer;
    int committing;//------------exec�ɂ�鏈�����s���Ă����1�A�s���̏d����h���B
    libvlc_event_manager_t* eventManager;
    libvlc_playback_mode_t playbackMode;//---------���[�v�Đ��A���s�[�g�Đ����A�f�t�H���g�Đ���
    std::string errorMess;
    long gHwnd;

public:
    static void CallbackNextFunc(const libvlc_event_t* event,void* arg);
    static void CallbackPlayedFunc(const libvlc_event_t* event,void* arg);
    vlcPlayer();
    int extensionMode;//--------------1�Ȃ�g���q���Ď����čĐ����邩��ʁB0���ƑS���Đ����悤�Ƃ���B
    std::vector<std::string> extensionList;//-------�Ή��g���q�̃��X�g
    int played;//---------------1�ōŏ��̋Ȃ��Đ������OnLoop�C�x���g�B�Đ��{�^���ɂ��Đ���0�ɂ����B
    std::vector<mediaData> mediaDataList;//-----------�v���C���X�g���̋ȃf�[�^�̔z��
    

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