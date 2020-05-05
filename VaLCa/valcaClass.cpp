#include "pch.h"

#include "csaori.h"
#include "vlcPlayer.h"

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

