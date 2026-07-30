#pragma once
#include <windows.h>
#include <vector>
#include <set>
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
    static std::set<std::string> getBlockedAppNames();

private:
    ApplicationManager() = default;
    static std::string RunCommandAndCaptureOutput(const std::string& command);
};
