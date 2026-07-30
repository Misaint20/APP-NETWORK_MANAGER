#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

// Helper: read a REG_SZ or REG_EXPAND_SZ value from the registry.
inline bool GetRegistryValueString(HKEY hKey, const std::string& valueName,
                                   std::string& valueOut) noexcept {
    DWORD type = 0;
    DWORD dataSize = 0;
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type,
                         nullptr, &dataSize) == ERROR_SUCCESS) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            std::vector<char> buffer(dataSize);
            if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type,
                    reinterpret_cast<LPBYTE>(buffer.data()),
                    &dataSize) == ERROR_SUCCESS) {
                valueOut = buffer.data();
                return true;
            }
        }
    }
    return false;
}

// Extract the parent folder from a path string.
// Handles the "C:\path\to\app.exe,0" format used by DisplayIcon values.
inline std::string ExtractFolderFromPath(const std::string& pathStr) noexcept {
    // Strip trailing ",0" or ",1" icon index
    size_t commaPos = pathStr.find(',');
    std::string cleanPath = (commaPos != std::string::npos)
        ? pathStr.substr(0, commaPos) : pathStr;

    std::filesystem::path p(cleanPath);
    if (p.has_parent_path())
        return p.parent_path().string();
    return {};
}

// Enumerate an uninstall registry key and invoke onApp() for each found app.
// This replaces 3 duplicate implementations (console ES, console EN, GUI).
// The callback receives (displayName, folderPath) for each application found.
inline void EnumerateUninstallKey(
    HKEY hKeyRoot, const std::string& subKey,
    const std::function<void(const std::string& name,
                             const std::string& folderPath)>& onApp) noexcept {

    HKEY hKey = nullptr;
    if (RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0,
                      KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
        return;

    char keyName[256];
    DWORD keyNameSize;
    DWORD index = 0;

    while (true) {
        keyNameSize = sizeof(keyName);
        if (RegEnumKeyExA(hKey, index++, keyName, &keyNameSize,
                          nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;

        HKEY hSubKey = nullptr;
        if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ,
                          &hSubKey) == ERROR_SUCCESS) {

            std::string displayName;
            if (GetRegistryValueString(hSubKey, "DisplayName", displayName)) {
                std::string folderPath;
                // First try InstallLocation
                if (!GetRegistryValueString(hSubKey, "InstallLocation",
                                             folderPath)) {
                    // Fall back to DisplayIcon and extract the folder
                    std::string displayIcon;
                    if (GetRegistryValueString(hSubKey, "DisplayIcon",
                                               displayIcon))
                        folderPath = ExtractFolderFromPath(displayIcon);
                }
                onApp(displayName, folderPath);
            }
            RegCloseKey(hSubKey);
        }
    }
    RegCloseKey(hKey);
}
