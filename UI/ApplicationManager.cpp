#include "ApplicationManager.h"
#include "../StringHelper.h"
#include "../RegistryHelper.h"
#include <algorithm>
#include <set>
#include <sstream>
using namespace std;
namespace fs = std::filesystem;

ApplicationManager& ApplicationManager::getInstance() {
    static ApplicationManager instance;
    return instance;
}

vector<ApplicationManager::Application> ApplicationManager::getInstalledApplications() {
    vector<Application> apps;
    apps.reserve(256);  // pre-allocate to avoid repeated reallocations
    {
        auto sink = [&](const string& name, const string& folderPath) {
            Application app;
            app.name = name;
            app.folderPath = folderPath;
            apps.push_back(std::move(app));
        };
        EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", sink);
        EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", sink);
        EnumerateUninstallKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", sink);
    }
    
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

// EnumerateUninstallKey moved to RegistryHelper.h (shared implementation)

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

// GetRegistryValueString and ExtractFolderFromPath moved to RegistryHelper.h (shared implementations)

// ── Firewall rule query helpers ──────────────────────────────────────────

string ApplicationManager::RunCommandAndCaptureOutput(const string& command) {
    string result;
    result.reserve(16384);  // typical netsh output is ~8-16 KB

    HANDLE hReadPipe  = INVALID_HANDLE_VALUE;
    HANDLE hWritePipe = INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };

    if(!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        return result;

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWritePipe;
    si.hStdError  = hWritePipe;

    // Convert command to UTF-16 for CreateProcessW
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), nullptr, 0);
    std::wstring wideCmd(static_cast<size_t>(wideLen > 0 ? wideLen : 0), L'\0');
    if (wideLen > 0)
        MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), &wideCmd[0], wideLen);

    std::vector<wchar_t> cmdLine(wideCmd.begin(), wideCmd.end());
    cmdLine.push_back(L'\0');

    PROCESS_INFORMATION pi = {};
    BOOL created = CreateProcessW(nullptr, cmdLine.data(),
        nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
        nullptr, nullptr, &si, &pi);

    // Close write end in parent so ReadFile doesn't hang
    CloseHandle(hWritePipe);
    hWritePipe = INVALID_HANDLE_VALUE;

    if(created) {
        char buffer[8192];
        DWORD bytesRead;
        while(ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result.append(buffer, bytesRead);
        }

        WaitForSingleObject(pi.hProcess, 10000);  // 10-second timeout
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    CloseHandle(hReadPipe);
    return result;
}

set<string> ApplicationManager::getBlockedAppNames() {
    set<string> blocked;

    // Query all outbound firewall rules (fast, single netsh call)
    string command = "netsh advfirewall firewall show rule name=all dir=out";
    string output = RunCommandAndCaptureOutput(command);

    if(output.empty())
        return blocked;

    // Locale-independent parsing: our rule names always start with "Block_"
    // regardless of Windows language, so we just search for that prefix.
    istringstream stream(output);
    string line;
    while(getline(stream, line)) {
        if(line.empty() || line[0] == '-') continue;

        size_t pos = line.find("Block_");
        if(pos != string::npos) {
            string name = line.substr(pos + 6);
            // Trim trailing whitespace and carriage return
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            if(!name.empty())
                blocked.insert(name);
        }
    }

    return blocked;
}
