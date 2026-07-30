#include <windows.h>
#include "../AdminHelper.h"
#include "MainWindow.h"

// Main application entry point
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