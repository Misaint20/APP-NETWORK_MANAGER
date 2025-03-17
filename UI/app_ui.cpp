#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include "MainWindow.h"
using namespace std;
namespace fs = std::filesystem;

// Structure for storing application information.
struct Application {
    string name;       // Display name
    string folderPath; // Installation path (not shown to user)
};

bool GetRegistryValueString(HKEY hKey, const string &valueName, string &valueOut) {
    DWORD type = 0, dataSize = 0;
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, nullptr, &dataSize) == ERROR_SUCCESS) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            vector<char> buffer(dataSize);
            if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type,
                reinterpret_cast<LPBYTE>(buffer.data()), &dataSize) == ERROR_SUCCESS) {
                valueOut = string(buffer.data());
                return true;
            }
        }
    }
    return false;
}

bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
         DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

string ExtractFolderFromPath(const string &pathStr) {
    size_t commaPos = pathStr.find(',');
    string cleanPath = (commaPos != string::npos) ? pathStr.substr(0, commaPos) : pathStr;
    fs::path p(cleanPath);
    if(p.has_parent_path())
        return p.parent_path().string();
    return "";
}

void EnumerateUninstallKey(HKEY hKeyRoot, const string &subKey, vector<Application>& apps) {
    HKEY hKey;
    if(RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        char keyName[256];
        DWORD keyNameSize;
        DWORD index = 0;
        while(true) {
            keyNameSize = sizeof(keyName);
            LONG ret = RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr);
            if(ret == ERROR_NO_MORE_ITEMS)
                break;
            if(ret == ERROR_SUCCESS) {
                HKEY hSubKey;
                if(RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    string displayName;
                    if(GetRegistryValueString(hSubKey, "DisplayName", displayName)) {
                        Application app;
                        app.name = displayName;
                        if(!GetRegistryValueString(hSubKey, "InstallLocation", app.folderPath)) {
                            string displayIcon;
                            if(GetRegistryValueString(hSubKey, "DisplayIcon", displayIcon))
                                app.folderPath = ExtractFolderFromPath(displayIcon);
                        }
                        apps.push_back(app);
                    }
                    RegCloseKey(hSubKey);
                }
                index++;
            }
            else break;
        }
        RegCloseKey(hKey);
    }
}

// Global vector to store installed applications.
vector<Application> gApps;

// Control IDs.
#define ID_LISTBOX 101
#define ID_BLOCK   102
#define ID_ALLOW   103
#define ID_EXIT    104

// Helper: returns first .exe file in the folder; returns empty string if none.
string GetExecutableFromFolder(const string &folderPath) {
    vector<string> exeFiles;
    try {
        for(const auto &entry: fs::directory_iterator(folderPath)) {
            if(entry.is_regular_file()) {
                fs::path p = entry.path();
                if(p.extension() == ".exe" || p.extension() == ".EXE")
                    exeFiles.push_back(p.string());
            }
        }
    } catch(...) { }
    if(!exeFiles.empty())
        return exeFiles[0];
    return "";
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hListBox;
    switch(message) {
        case WM_CREATE: {
            // Create a list box for installed applications.
            hListBox = CreateWindowA("LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | LBS_STANDARD,
                10, 10, 400, 200, hwnd, (HMENU)ID_LISTBOX, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            // Create Block, Allow, and Exit buttons.
            CreateWindowA("BUTTON", "Block",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, 220, 100, 30, hwnd, (HMENU)ID_BLOCK, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindowA("BUTTON", "Allow",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                120, 220, 100, 30, hwnd, (HMENU)ID_ALLOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CreateWindowA("BUTTON", "Exit",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                230, 220, 100, 30, hwnd, (HMENU)ID_EXIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            // Populate list box with application names.
            for(auto &app : gApps) {
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)app.name.c_str());
            }
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if(wmId == ID_BLOCK || wmId == ID_ALLOW) {
                int sel = (int)SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
                if(sel == LB_ERR) {
                    MessageBoxA(hwnd, "Please select an application.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                Application selectedApp = gApps[sel];
                string exePath = GetExecutableFromFolder(selectedApp.folderPath);
                if(exePath.empty()) {
                    MessageBoxA(hwnd, "No executable found in the selected application's folder.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                string ruleName = "Block_" + selectedApp.name;
                string command;
                if(wmId == ID_BLOCK) {
                    command = "netsh advfirewall firewall add rule name=\"" + ruleName + "\" dir=out action=block program=\"" + exePath + "\" enable=yes";
                } else {
                    command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";
                }
                system(command.c_str());
                MessageBoxA(hwnd, "Action executed.", "Info", MB_OK | MB_ICONINFORMATION);
            } else if(wmId == ID_EXIT) {
                PostQuitMessage(0);
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if(!IsRunAsAdmin()) {
        char exePath[MAX_PATH];
        if(GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
            ShellExecuteA(NULL, "runas", exePath, GetCommandLineA(), NULL, SW_SHOWNORMAL);
        }
        return 0;
    }
    
    MainWindow& mainWindow = MainWindow::getInstance();
    if(!mainWindow.create(hInstance)) {
        MessageBoxA(nullptr, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    mainWindow.show(nCmdShow);
    
    MSG msg = {};
    while(GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}