#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <limits>
#include "AdminHelper.h"
#include "RegistryHelper.h"
#include "FirewallHelper.h"
#include "StringHelper.h"
using namespace std;
namespace fs = std::filesystem;

// Structure to store application information.
struct Application {
    string name;       // Displayed name
    string folderPath; // Installation path (not shown to user)
};

// Shared helper implementations are now in AdminHelper.h and RegistryHelper.h

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
    // Scan registry using the shared helper
    auto scanKey = [&](HKEY root, const char* keyPath) {
        EnumerateUninstallKey(root, keyPath, [&](const string& name, const string& folderPath) {
            apps.push_back({name, folderPath});
        });
    };
    scanKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    scanKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    scanKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    
    // Remove duplicates (based on name) and sort.
    sort(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name < b.name;
    });
    apps.erase(unique(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name == b.name;
    }), apps.end());
    
    if (apps.empty()) {
        cout << "No installed applications found." << endl;
        cout << "Press Enter to exit...";
        cin.get();
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
                cout << "Press Enter to exit...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); cin.get();
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
        string safeName = SanitizeForCmd(selectedApp.name);
        string safeExe = SanitizeForCmd(targetExe);
        string ruleName = "Block_" + safeName;
        string command;
        if (action == 1) {
            command = "netsh advfirewall firewall add rule name=\"" + ruleName +
                      "\" dir=out action=block program=\"" + safeExe + "\" enable=yes";
        } else if (action == 2) {
            command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";
        }
        
        cout << "\nExecuting firewall command..." << endl;
        if(ExecuteFirewallCommand(command) == 0) {
            cout << "✓ Operation completed successfully." << endl;
        } else {
            cout << "✗ Error: Could not complete the operation. "
                    "Make sure you are running as Administrator." << endl;
        }
        
        cout << "\nPress Enter to return to the application list..." << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); cin.get();
    }
}