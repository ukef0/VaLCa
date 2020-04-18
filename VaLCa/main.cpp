#include "pch.h"
#include <windows.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include <string>
#include "csaori.h"
typedef SSIZE_T ssize_t;
#include <vlc/vlc.h>
#define NUM_META 5

class vlcPlayer {
private:
    //char* path;
    static libvlc_instance_t* instance;
    static libvlc_media_t* media;
    static libvlc_media_player_t* media_player;
    static int flug;
    
    
public:
    static int playable;
    static std::string meta[];
    vlcPlayer();
    static int getFlug();

    static void initialize();
    static void setMedia(const char* input_path);
    static void play();
    static bool isPlaying();
    static libvlc_time_t getTime();
    static void stop();
    static void release();
    static void pauseToggle();
};
libvlc_instance_t* vlcPlayer::instance;
libvlc_media_t* vlcPlayer::media;
libvlc_media_player_t* vlcPlayer::media_player;
int vlcPlayer::flug;
int vlcPlayer::playable;
std::string vlcPlayer::meta[NUM_META];
std::string toStr(const char* str) {
    if (str == NULL)
        return "__NULL__";
    else
        return str;
}
std::string timeStr(libvlc_time_t time) {
    char timeStr[19] = "";
    sprintf_s(timeStr, 19, "%lld", time);
    return toStr(timeStr);
}

void vlcPlayer::initialize() {
    instance = libvlc_new(0, NULL);
    flug = 0;
    playable = 0;
}
void vlcPlayer::setMedia(const char* input_path) {
    if (flug != 0) {
        libvlc_media_player_release(media_player);
    }
    media = libvlc_media_new_path(instance, input_path);
    if (libvlc_media_parse_with_options(media, libvlc_media_fetch_local, 5000) == 0) {
        Sleep(100);
        playable = 1;
        int64_t time=libvlc_media_get_duration(media);
        meta[0] = toStr(libvlc_media_get_meta(media, libvlc_meta_Title));
        meta[1] = toStr(libvlc_media_get_meta(media, libvlc_meta_Artist));
        meta[2] = toStr(libvlc_media_get_meta(media, libvlc_meta_Album));
        meta[3] = toStr(libvlc_media_get_meta(media, libvlc_meta_TrackNumber));
        /*char timeStr[19]="";
        sprintf_s(timeStr,19,"%lld", time);*/
        meta[4] = timeStr(time);

        media_player = libvlc_media_player_new_from_media(media);
        flug = 1;
    }
    else {
        playable = 0;
    }
    libvlc_media_release(media);
    
}
vlcPlayer::vlcPlayer() {
}
int vlcPlayer::getFlug() {
    return flug;
}
void vlcPlayer::play() {
    if (getFlug()&&playable) {
        libvlc_media_player_play(media_player);
    }
}
libvlc_time_t vlcPlayer::getTime() {
    if (getFlug() &&isPlaying())
        return libvlc_media_player_get_time(media_player);
    else
        return 0;
}
bool vlcPlayer::isPlaying() {
    if (getFlug())
        return libvlc_state_t::libvlc_Ended != libvlc_media_player_get_state(media_player)&&libvlc_state_t::libvlc_Stopped != libvlc_media_player_get_state(media_player);
    else
        return false;
}
void vlcPlayer::stop() {
    if (getFlug()) 
        libvlc_media_player_stop(media_player);
}
void vlcPlayer::pauseToggle() {
    if (getFlug()&&libvlc_state_t::libvlc_Playing== libvlc_media_player_get_state(media_player)) {
        libvlc_media_player_pause(media_player);
    }
    else {
        vlcPlayer::play();
    }
}
void vlcPlayer::release() {
    if (getFlug())
        libvlc_media_player_release(media_player);
    playable = 0;
    libvlc_release(instance);
}

bool CSAORI::load() {
    //playerインスタンスの作成
    vlcPlayer::initialize();
    return true;
}
bool CSAORI::unload() {
    if (vlcPlayer::isPlaying())
        vlcPlayer::stop();
    vlcPlayer::release();
    return true;
}

void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
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
            /*char timeStr[19]="";
            sprintf_s(timeStr, 19, "%lld", vlcPlayer::getTime());*/
            
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
}

