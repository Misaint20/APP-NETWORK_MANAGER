#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>

class ApplicationManager {
public:
    struct Application {
        std::string name;
        std::string folderPath;
        std::vector<std::string> executables;
    };

    static ApplicationManager& getInstance();
    
    std::vector<Application> getInstalledApplications();
    bool blockInternet(const Application& app, const std::string& exePath);
    bool allowInternet(const Application& app, const std::string& exePath);
    std::vector<std::string> findExecutables(const std::string& folderPath);

private:
    ApplicationManager() = default;
    bool GetRegistryValueString(HKEY hKey, const std::string& valueName, std::string& valueOut);
    void EnumerateUninstallKey(HKEY hKeyRoot, const std::string& subKey, std::vector<Application>& apps);
    std::string ExtractFolderFromPath(const std::string& pathStr);
};
