#pragma once
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include <string>
#include <fstream>
#include <iostream>
#  include <basetsd.h>
//typedef SSIZE_T ssize_t;
#define MY_MAX_PATH 1000
#include <map>

#include <filesystem>
#include <algorithm>
#include <time.h>
#include <random>
#include <regex>
//#undef OpenFileMapping 
#include <windows.h>
#include <vlc/vlc.h>
#include <mutex>
#ifndef ___VALCA_UTIL___
#define ___VALCA_UTIL___
typedef struct media_data {
    std::string path;
    std::string albumName;
    std::string title;
    std::string artist;
    int discNumber;
    int discTotal;
    int trackTotal;
    int64_t length;
    int trackNumber;
}mediaData;
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

//

namespace VaLCaFunc {

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
}
#endif