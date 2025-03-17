#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <shellapi.h>
#include <sddl.h>
using namespace std;
namespace fs = std::filesystem;

// Structure to store application information.
struct Application {
    string name;       // Displayed name
    string folderPath; // Installation path (not shown to user)
};

// Helper function to get a string value from the registry.
bool GetRegistryValueString(HKEY hKey, const string &valueName, string &valueOut) {
    DWORD type = 0;
    DWORD dataSize = 0;
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, nullptr, &dataSize) == ERROR_SUCCESS) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            vector<char> buffer(dataSize);
            if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(buffer.data()), &dataSize) == ERROR_SUCCESS) {
                valueOut = string(buffer.data());
                return true;
            }
        }
    }
    return false;
}

// Function to check if the program is running with administrator privileges.
bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

// Function to extract the folder from a string that may include an executable and sometimes a comma 
// (e.g., "C:\Program Files\App\App.exe,0").
string ExtractFolderFromPath(const string &pathStr) {
    size_t commaPos = pathStr.find(',');
    string cleanPath = (commaPos != string::npos) ? pathStr.substr(0, commaPos) : pathStr;
    fs::path p(cleanPath);
    if (p.has_parent_path())
        return p.parent_path().string();
    return "";
}

// Function to enumerate an uninstall registry key and add the found applications.
void EnumerateUninstallKey(HKEY hKeyRoot, const string &subKey, vector<Application>& apps) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        char keyName[256];
        DWORD keyNameSize;
        DWORD index = 0;
        while (true) {
            keyNameSize = sizeof(keyName);
            LONG ret = RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr);
            if (ret == ERROR_NO_MORE_ITEMS)
                break;
            if (ret == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    string displayName;
                    if (GetRegistryValueString(hSubKey, "DisplayName", displayName)) {
                        Application app;
                        app.name = displayName;
                        // First, try to get "InstallLocation"
                        if (!GetRegistryValueString(hSubKey, "InstallLocation", app.folderPath)) {
                            // If not available, try "DisplayIcon" and extract the folder.
                            string displayIcon;
                            if (GetRegistryValueString(hSubKey, "DisplayIcon", displayIcon))
                                app.folderPath = ExtractFolderFromPath(displayIcon);
                        }
                        apps.push_back(app);
                    }
                    RegCloseKey(hSubKey);
                }
                index++;
            } else {
                break;
            }
        }
        RegCloseKey(hKey);
    }
}

// Modificación de main para aceptar parámetros y solicitar elevación si es necesario.
int main(int argc, char* argv[]) {
    if (!IsRunAsAdmin()) {
        char exePath[MAX_PATH];
        if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
            ShellExecuteA(NULL, "runas", exePath, GetCommandLineA(), NULL, SW_SHOWNORMAL);
        }
        return 0;
    }
    
    // Set the window title.
    SetConsoleTitleA("NETCONTROL-APP - Internet Access Manager");
    
    // Display loading message.
    cout << "Loading applications, please wait..." << endl;
    
    vector<Application> apps;
    // Se consultan varias claves del registro para obtener las aplicaciones instaladas.
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    
    // Remove duplicates (based on name) and sort.
    sort(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name < b.name;
    });
    apps.erase(unique(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name == b.name;
    }), apps.end());
    
    if (apps.empty()) {
        cout << "No installed applications found." << endl;
        system("pause");
        return 0;
    }
    
    while (true) { // Main loop to return to the application list after an action.
        // --- Application selection ---
        Application selectedApp;
        while (true) {
            cout << "\nInstalled Applications:" << endl;
            for (size_t i = 0; i < apps.size(); i++) {
                cout << i + 1 << ". " << apps[i].name << endl;
            }
            cout << "0. Exit" << endl;
            cout << "\nSelect the application number: ";
            int choice;
            cin >> choice;
            if (choice == 0) {
                cout << "Exiting the program." << endl;
                system("pause");
                return 0;
            }
            if (choice < 1 || choice > static_cast<int>(apps.size())) {
                cout << "Invalid option." << endl;
                continue;
            }
            selectedApp = apps[choice - 1];
            cout << "You have selected: " << selectedApp.name << endl;
            cout << "\nSelect:" << endl;
            cout << "1. Continue with this application" << endl;
            cout << "2. Return to the application list" << endl;
            int confirm;
            cin >> confirm;
            if (confirm == 1) {
                break;
            }
            // If option 2, return to the list.
        }
        
        // If installation path not found, request it manually.
        if (selectedApp.folderPath.empty()) {
            cout << "Installation path for this application was not found." << endl;
            cout << "Enter the folder path manually: ";
            cin.ignore();
            getline(cin, selectedApp.folderPath);
        }
        
        // --- Executable selection ---
        vector<string> exeFiles;
        try {
            for (const auto &entry : fs::directory_iterator(selectedApp.folderPath)) {
                if (entry.is_regular_file()) {
                    fs::path p = entry.path();
                    if (p.extension() == ".exe" || p.extension() == ".EXE")
                        exeFiles.push_back(p.string());
                }
            }
        } catch (...) {
            // Si ocurre algún error al acceder a la carpeta.
        }
        
        string targetExe;
        if (exeFiles.empty()) {
            cout << "No .exe files were found in the folder." << endl;
            cout << "Enter the full path of the executable manually: ";
            cin.ignore();
            getline(cin, targetExe);
        } else if (exeFiles.size() == 1) {
            targetExe = exeFiles[0];
            cout << "A single executable was detected in the folder." << endl;
        } else {
            cout << "Multiple executables were found in the folder:" << endl;
            for (size_t i = 0; i < exeFiles.size(); i++) {
                cout << i + 1 << ". " << exeFiles[i] << endl;
            }
            cout << "Select the number of the executable you want to use: ";
            int exeChoice;
            cin >> exeChoice;
            if (exeChoice < 1 || exeChoice > static_cast<int>(exeFiles.size())) {
                cout << "Invalid option. The first executable found will be used." << endl;
                targetExe = exeFiles[0];
            } else {
                targetExe = exeFiles[exeChoice - 1];
            }
        }
        
        cout << "\nSelected executable path: " << targetExe << endl;
        
        // --- Action selection ---
        int action = 0;
        while (true) {
            cout << "\nSelect action:" << endl;
            cout << "1. Block Internet Access" << endl;
            cout << "2. Allow Internet Access" << endl;
            cout << "3. Return to the application list" << endl;
            cout << "Enter your option: ";
            cin >> action;
            if (action == 1 || action == 2 || action == 3) {
                break;
            }
            cout << "Invalid option. Please try again." << endl;
        }
        
        if (action == 3) {
            continue;
        }
        
        // Define a firewall rule name based on the application name.
        string ruleName = "Block_" + selectedApp.name;
        string command;
        if (action == 1) {
            command = "netsh advfirewall firewall add rule name=\"" + ruleName +
                      "\" dir=out action=block program=\"" + targetExe + "\" enable=yes";
        } else if (action == 2) {
            command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";
        }
        
        cout << "\nExecuting command: " << command << endl;
        system(command.c_str());
        
        cout << "\nAction executed. Press any key to return to the application list." << endl;
        system("pause");
    }
}