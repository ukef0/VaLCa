#include "pch.h"
#include "csaori_base.h"
#define NUM_META 5



#include <inttypes.h>
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
#include <vlc/vlc.h>
#include <mutex>
#define NUM_KEY 8
#include "VaLCa_util.h"



namespace VaLCaFunc {
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
        {// elem‚Ísub_match
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
}