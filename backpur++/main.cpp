//
//
//
#define CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>
#include <filesystem> 

//
using namespace std;
namespace fs = std::filesystem;
HHOOK g_hook;

void showNotification(const std::wstring& title, const std::wstring& message) {
    MessageBoxW(NULL, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}
std::string getCurrentTime() {
    auto currentTime = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(currentTime);
    struct tm timeStruct;
    localtime_s(&timeStruct, &time);
    std::stringstream ss;
    ss << std::put_time(&timeStruct, "%Y%m%d-%H%M%S");
    return ss.str();
}

void copy(const std::string& folderName) {
    try
    {
        std::string sourceFolder = "C:\\Users\\dhima\\AppData\\Local\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\LocalState\\games\\com.mojang\\minecraftWorlds\\ir+bZCOnBgA=";
        std::string destinationFolder = "D:\\MinecraftBackup\\" + folderName;
        fs::copy(sourceFolder, destinationFolder, fs::copy_options::recursive | fs::copy_options::overwrite_existing);    
    }
    catch (exception ex) {
        std::wstring title = L"Error";
        std::wstring message = L"Failed to copy";
        showNotification(title, message);
    }
}
void backuper() {
    std::string folderName = "ir+bZCOnBgA=" + getCurrentTime();
    std::string newPath = "D:\\MinecraftBackup";
    std::wstring folder = std::wstring(folderName.begin(), folderName.end());
    std::wstring path = std::wstring(newPath.begin(), newPath.end());
    std::wstring fullPath = path + L"\\" + folder;
    if (CreateDirectory(fullPath.c_str(), nullptr)) {
        //std::cout << "Folder created successfully." << std::endl;
        std::cout << "  'ir+bZCOnBgA=  Sucessfully Copied.'   " << std::endl;
        copy(folderName);
    }
    else {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            std::cout << "Folder already exists." << std::endl;
        }
        else {
            std::cout << "Failed to create folder. Error code: " << error << std::endl;
        }
    }
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN)
        {
            if (pKeyboardStruct->vkCode == VK_ESCAPE && GetAsyncKeyState(VK_LWIN) < 0)
            {
                // Exit the program
                PostQuitMessage(0);
            }
            else if (pKeyboardStruct->vkCode == VK_OEM_3)
            {
                // Print "hello"
                //std::cout << "hello" << std::endl;
                std::cout << getCurrentTime() ;
                backuper();
            }
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

int main()
{
    // Install the keyboard hook
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    if (g_hook == NULL)
    {
        std::cout << "Failed to install hook" << std::endl;
        return 1;
    }

    // Message loop to keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Uninstall the hook
    UnhookWindowsHookEx(g_hook);

    return 0;
}