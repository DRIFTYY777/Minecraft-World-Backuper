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
#include <ShlObj.h>
#include <atlbase.h>
#include <conio.h>
#include <fstream>
#include <locale>
#include <codecvt>

//
using namespace std;
namespace fs = std::filesystem;
HHOOK g_hook;

/* Check file exists or not*/
bool fileExists(const std::string& filename) {
    std::ifstream file(filename, std::ios::in);
    return file.is_open(); // Check if the file was opened successfully
}

/*---------------------------------String Converter---------------------------------*/
std::wstring stringToWstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

/* Show Message Box*/
void message_box(const std::string& title, const std::string& message) {
    std::wstring wtitle = stringToWstring(title);
    std::wstring wmessage = stringToWstring(message);

    int msgboxID = MessageBox(
        NULL,
        wmessage.c_str(),
        wtitle.c_str(),
        MB_ICONINFORMATION | MB_OK
    );
}

/*----------------------------------------------------folder picker----------------------------------------------------*/
#ifdef __cplusplus  
extern "C" {
#endif
    struct ComInit {
        ComInit()
        {
            CoInitialize(nullptr);
        }
        ~ComInit()
        {
            CoUninitialize();
        }
    };
    std::wstring folder_selector() {
        //init COM to be able to use classes 
        ComInit com;
        CComPtr<IFileOpenDialog> pFolderDlg;
        pFolderDlg.CoCreateInstance(CLSID_FileOpenDialog);
        FILEOPENDIALOGOPTIONS options{};
        pFolderDlg->GetOptions(&options);
        pFolderDlg->SetOptions(options | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);
        if (SUCCEEDED(pFolderDlg->Show(nullptr)))
        {
            CComPtr<IShellItem> pSelectedItem;
            pFolderDlg->GetResult(&pSelectedItem);
            CComHeapPtr<wchar_t> pPath;
            pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
            std::wcout << pPath.m_pData << std::endl;
            // Clean up COM
            CoUninitialize();
            return pPath.m_pData;
        }
        else
        {
            std::wcout << "No folder selected" << std::endl;
            message_box("Error", "No folder selected");
            return L"";
        }
    }
#ifdef __cplusplus  
}
#endif

/*------------------------------------Target Folder path save to File------------------------------------*/
void data_logger(const std::string& file_name, const std::string& data_to_add) {
    // Check if data is empty (string) or zero (numeric conversion)
    if (data_to_add.empty() || data_to_add == "0") {
        std::cout << "Data is empty or zero. Not logging." << std::endl;
        return;
    }
    // Open the file in truncate mode (clears existing data)
    std::ofstream file(file_name, std::ios::out | std::ios::trunc);
    // Check if the file was opened successfully
    if (file.is_open()) {
        // Write data to the file
        file << data_to_add << std::endl;
        std::cout << "Data written to data.txt" << std::endl;
    }
    else {
        std::cerr << "Error opening file data.txt" << std::endl;
        message_box("Error", "Error opening file data.txt");
    }
}
/*---------------------------------String Converter---------------------------------*/
std::string wstringToString(const std::wstring& wide_string) {
    if (wide_string.empty()) return std::string();
    // Get the size of the buffer required for the conversion
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_string[0], (int)wide_string.size(), NULL, 0, NULL, NULL);
    // Create a string with the necessary size
    std::string narrow_string(size_needed, 0);
    // Perform the conversion
    WideCharToMultiByte(CP_UTF8, 0, &wide_string[0], (int)wide_string.size(), &narrow_string[0], size_needed, NULL, NULL);
    return narrow_string;
}

/*---------------------------------Get Current Time---------------------------------*/
std::string getCurrentTime() {
    auto currentTime = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(currentTime);
    struct tm timeStruct;
    localtime_s(&timeStruct, &time);
    std::stringstream ss;
    ss << std::put_time(&timeStruct, "%Y%m%d-%H%M%S");
    return ss.str();
}

/*Copying Source folder to Backup place*/
void copy(const std::string& sourceFolder, const std::string& destinationFolder, const std::string& folderName) {
    try
    {
        std::cout << sourceFolder << std::endl;
        std::cout << destinationFolder << std::endl;
        std::cout << folderName << std::endl;
        std::cout << destinationFolder +"\\" + folderName << std::endl;
        fs::copy(sourceFolder, destinationFolder +"\\" + folderName, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }
    catch (exception ex) {
        message_box("Error", "Failed to copy");
    }
}

/*---------------------------------Get Last Folder Name---------------------------------*/
std::string getLastFolderName(const std::string& path) {
    // Find the position of the last backslash
    size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) {
        // No backslash found, return the whole path as it is
        return path;
    }
    // Return the substring after the last backslash
    return path.substr(pos + 1);
}

/*---------------------------------Get Data from File---------------------------------*/
std::string getdatafromfile(const std::string& file_name) {
	std::ifstream file(file_name);
	std::string data;
	std::string line;
    while (std::getline(file, line)) {
		data += line;
	}
	file.close();
	return data;
}

/*Allocating path for backups*/
void backuper() {
    std::string data = getdatafromfile("data.txt");
    std::string data2 = getdatafromfile("data2.txt");
    std::string folderName = getLastFolderName(data) +" "+ getCurrentTime();
    std::string newPath = data2 + "MinecraftBackup";
    if (!std::filesystem::exists(newPath)) {
        try {
            std::filesystem::create_directory(newPath);
            std::cout << "Folder created successfully: " << newPath << std::endl;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating folder: " << e.what() << std::endl;
            message_box("Error", "Error creating folder");
        }
    }
    std::wstring folder = std::wstring(folderName.begin(), folderName.end());
    std::wstring path = std::wstring(newPath.begin(), newPath.end());
    std::wstring fullPath = path + L"\\" + folder;
    if (CreateDirectory(fullPath.c_str(), nullptr)) {
        //std::cout << "Folder created successfully." << std::endl;
        std::cout << "Sucessfully Copied." << std::endl;
        copy(data, newPath, folderName);
    }
    else {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            std::cout << "Folder already exists." << std::endl;
        }
        else {
            std::cout << "Failed to create folder. Error code: " << error << std::endl;
            message_box("Error", "Failed to create folder");
        }
    }
}

/*---------------------------------Keyboard Hook---------------------------------*/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN)
        {
            if (pKeyboardStruct->vkCode == VK_ESCAPE && GetAsyncKeyState(VK_LWIN) < 0)
            {
                // WIN + ESC
                PostQuitMessage(0);
            }
            else if (pKeyboardStruct->vkCode == 0x50 && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_SHIFT) < 0)
            {
                //ctrl + shift + p

				std::string converted_data = wstringToString(folder_selector());
				data_logger("data.txt", converted_data);
			}
            else if (pKeyboardStruct->vkCode == 0x4C && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_SHIFT) < 0)
            {
                //ctrl + shift + l
                std::string converted_data = wstringToString(folder_selector());
                data_logger("data2.txt", converted_data);
				
			}
            else if (pKeyboardStruct->vkCode == VK_OEM_3)
            {
               // SHIFT + `
                if (!fileExists("data.txt")) {
                    message_box("Select Folder", "Select the folder you want to backup");
                    std::string converted_data = wstringToString(folder_selector());
                    data_logger("data.txt", converted_data);
                }
                else
                {
                    backuper();
                }
            }
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

/*MAIN FUNCTION*/
int main()
{
    std::cerr << "'Ctrl + Shift + P' for selecting folder for backup" << std::endl;
    std::cerr << "'Ctrl + Shift + L' for saving location for backup" << std::endl;
    std::cerr << "'Shift + ~' for creating backup" << std::endl;
    std::cerr << "'Win + Esc' for Exit" << std::endl;
    std::cerr << std::endl << std::endl;

    if (!fileExists("data.txt")) {
        message_box("Select Folder", "Select the folder you want to backup");
        std::string converted_data = wstringToString(folder_selector());
        data_logger("data.txt", converted_data);
	}

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
