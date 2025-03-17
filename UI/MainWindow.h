#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "ApplicationManager.h"

class MainWindow {
public:
    static MainWindow& getInstance();
    bool create(HINSTANCE hInstance);
    void show(int nCmdShow);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    MainWindow() = default;
    void createControls(HWND hwnd);
    void updateLayout(HWND hwnd);
    void onSize(HWND hwnd, UINT flag, int width, int height);
    void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void refreshApplicationList();

    HWND m_hwnd = nullptr;
    HWND m_hwndList = nullptr;
    HWND m_hwndBlockBtn = nullptr;
    HWND m_hwndAllowBtn = nullptr;
    HWND m_hwndExitBtn = nullptr;
    std::vector<ApplicationManager::Application> m_apps;

    // Control IDs
    static constexpr int ID_LISTBOX = 101;
    static constexpr int ID_BLOCK = 102;
    static constexpr int ID_ALLOW = 103;
    static constexpr int ID_EXIT = 104;
};
