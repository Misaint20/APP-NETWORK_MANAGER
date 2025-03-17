#include "ApplicationManager.h"
#include <algorithm>
using namespace std;
namespace fs = std::filesystem;

ApplicationManager& ApplicationManager::getInstance() {
    static ApplicationManager instance;
    return instance;
}

vector<ApplicationManager::Application> ApplicationManager::getInstalledApplications() {
    vector<Application> apps;
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    
    sort(apps.begin(), apps.end(), 
        [](const Application& a, const Application& b) { return a.name < b.name; });
    apps.erase(unique(apps.begin(), apps.end(),
        [](const Application& a, const Application& b) { return a.name == b.name; }), apps.end());
    
    // Pre-scan executables for each application
    for(auto& app : apps) {
        if(!app.folderPath.empty()) {
            app.executables = findExecutables(app.folderPath);
        }
    }
    
    return apps;
}

void ApplicationManager::EnumerateUninstallKey(HKEY hKeyRoot, const string& subKey, vector<Application>& apps) {
    HKEY hKey;
    if(RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        char keyName[256];
        DWORD keyNameSize;
        DWORD index = 0;
        
        while(true) {
            keyNameSize = sizeof(keyName);
            if(RegEnumKeyExA(hKey, index++, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                break;
                
            HKEY hSubKey;
            if(RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                string displayName;
                if(GetRegistryValueString(hSubKey, "DisplayName", displayName)) {
                    Application app;
                    app.name = displayName;
                    GetRegistryValueString(hSubKey, "InstallLocation", app.folderPath);
                    apps.push_back(app);
                }
                RegCloseKey(hSubKey);
            }
        }
        RegCloseKey(hKey);
    }
}

vector<string> ApplicationManager::findExecutables(const string& folderPath) {
    vector<string> executables;
    try {
        for(const auto& entry : fs::directory_iterator(folderPath)) {
            if(entry.is_regular_file() && entry.path().extension() == ".exe") {
                executables.push_back(entry.path().string());
            }
        }
    } catch(...) {}
    return executables;
}

bool ApplicationManager::GetRegistryValueString(HKEY hKey, const string& valueName, string& valueOut) {
    DWORD type = 0;
    DWORD dataSize = 0;
    
    if(RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, nullptr, &dataSize) == ERROR_SUCCESS) {
        if(type == REG_SZ || type == REG_EXPAND_SZ) {
            vector<char> buffer(dataSize);
            if(RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, 
                reinterpret_cast<LPBYTE>(buffer.data()), &dataSize) == ERROR_SUCCESS) {
                valueOut = buffer.data();
                return true;
            }
        }
    }
    return false;
}
