#include <iostream>
#include <sstream>

#define EZPW_OnWindows (defined(_WIN32) || defined(WIN32))
#if(EZPW_OnWindows)
    //Native Win32 clipboard functions
    #include <windows.h>
    #include <winuser.h>
#else
    //Try to fallback on libclipboard for non-windows platforms. Works for X11 linux (-lxcb), don't know about macOS.
    #include <libclipboard.h>
    clipboard_c* cb = clipboard_new(NULL);
#endif

std::string charList = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%&_";

bool isGoodPassword(std::string pw)
{
    if(pw.size()<16) return false;

    std::string lc = charList.substr(0, 26);    int numLCs = 0;
    std::string uc = charList.substr(26, 26);   int numUCs = 0;
    std::string num = charList.substr(52, 10);  int numNUMs = 0;
    std::string sym = charList.substr(62);      int numSYMs = 0;

    for(int i = 0; i<pw.size(); i++) {
        if(lc .find(pw[i])!=std::string::npos) { numLCs++; continue; }
        if(uc .find(pw[i])!=std::string::npos) { numUCs++; continue; }
        if(num.find(pw[i])!=std::string::npos) { numNUMs++; continue; }
        if(sym.find(pw[i])!=std::string::npos) { numSYMs++; continue; }
    }

    if(numLCs>2 && numUCs>2 && numNUMs>2 && numSYMs>2) {
        return true;
    }
    return false;
}

std::string genGoodPassword()
{
    std::string res = "???null???";
    do {
        std::stringstream ss;
        for(int c = 0; c<16; c++) {
            int idx = rand()%charList.size();
            ss << charList[idx];
        }
        res = ss.str();
    } while(!isGoodPassword(res));

    return res;
}

void setClipboard(std::string toCopy)
{
    #if(EZPW_OnWindows)
        const char* output = toCopy.c_str();
        const size_t len = strlen(output) + 1;
        HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
        memcpy(GlobalLock(hMem), output, len);
        GlobalUnlock(hMem);
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    #else
        clipboard_set_text(cb, toCopy.c_str());
    #endif
}

int main(int argc, char** args)
{
    //Seed securely
    std::srand(time(NULL));
    int seed = rand();
    for(int i = 0; i<612; i++) {
        std::srand(seed);
        seed = rand();
    }

    //Logic    
    bool running = true;
    int attempts = 1;

    printf("Press Enter to generate a new password (this will copy it to your clipboard automatically).\n");
    printf("Press Ctrl+C to exit.\n");

    while(running) {
        std::string userIn;
        std::getline(std::cin, userIn);

        std::string newPass = genGoodPassword();
        setClipboard(newPass);

        printf("#%d: %s", attempts, newPass.c_str());

        attempts++;
    }
    
    #if(!EZPW_OnWindows)
        clipboard_free(cb);
    #endif
    return 0;
}

#if(EZPW_OnWindows)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    char** x = new char*[1];
    return main(0, x);
}
#endif