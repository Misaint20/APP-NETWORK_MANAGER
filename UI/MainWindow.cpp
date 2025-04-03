#define UNICODE
#define _UNICODE
#include "MainWindow.h"
#include <windowsx.h>
#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

MainWindow& MainWindow::getInstance() {
    static MainWindow instance;
    return instance;
}

bool MainWindow::create(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"NetControlWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // Cargar el ícono usando el ID definido en resources.rc
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));    // Ícono grande
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));  // Ícono pequeño

    if (!RegisterClassEx(&wc))
        return false;

    // Create window with extended style for better visual
    m_hwnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW | WS_EX_CONTROLPARENT,
        L"NetControlWindow",
        L"APP-NETWORK_MANAGER",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 400, 
        nullptr, nullptr, hInstance, this
    );

    return (m_hwnd != nullptr);
}

void MainWindow::createControls(HWND hwnd) {
    // Create controls with modern style
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Create list view instead of list box for better visual and performance
    m_hwndList = CreateWindowEx(
        0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 0, 0, hwnd, (HMENU)101,
        hInst, nullptr);
        
    // Add columns to list view
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 450;  
    WCHAR columnName[] = L"Application Name";
    lvc.pszText = columnName;
    ListView_InsertColumn(m_hwndList, 0, &lvc);
    
    // Crear botones con estilo moderno y mejores dimensiones
    const wchar_t* buttonStyle = L"button";
    DWORD btnStyle = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER;
    
    m_hwndBlockBtn = CreateWindowEx(
        0, buttonStyle, L"Block Internet",
        btnStyle,
        0, 0, 0, 0, hwnd, (HMENU)102,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), nullptr);

    m_hwndAllowBtn = CreateWindowEx(
        0, buttonStyle, L"Allow Internet",
        btnStyle,
        0, 0, 0, 0, hwnd, (HMENU)103,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), nullptr);

    m_hwndExitBtn = CreateWindowEx(
        0, buttonStyle, L"Exit",
        btnStyle,
        0, 0, 0, 0, hwnd, (HMENU)104,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), nullptr);

    // Establecer la fuente para los botones
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessage(m_hwndBlockBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hwndAllowBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hwndExitBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hwndList, WM_SETFONT, (WPARAM)hFont, TRUE);

    refreshApplicationList();
}

void MainWindow::updateLayout(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    const int padding = 8;  
    const int buttonHeight = 25; 
    const int buttonWidth = 100;  
    
    // Ajustar ListView
    SetWindowPos(m_hwndList, nullptr,
        padding, padding,
        rc.right - 2*padding,
        rc.bottom - 2*padding - buttonHeight,
        SWP_NOZORDER);
    
    // Centrar botones en la parte inferior
    int totalButtonWidth = (buttonWidth * 3) + (padding * 2);
    int startX = (rc.right - totalButtonWidth) / 2;
    int y = rc.bottom - padding - buttonHeight;
    
    SetWindowPos(m_hwndBlockBtn, nullptr,
        startX, y, buttonWidth, buttonHeight, SWP_NOZORDER);
    
    SetWindowPos(m_hwndAllowBtn, nullptr,
        startX + buttonWidth + padding, y, buttonWidth, buttonHeight, SWP_NOZORDER);
    
    SetWindowPos(m_hwndExitBtn, nullptr,
        startX + (buttonWidth + padding) * 2, y, buttonWidth, buttonHeight, SWP_NOZORDER);
        
    InvalidateRect(hwnd, nullptr, TRUE);
}

void MainWindow::show(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}

void MainWindow::refreshApplicationList() {
    ListView_DeleteAllItems(m_hwndList);
    m_apps = ApplicationManager::getInstance().getInstalledApplications();
    
    for(size_t i = 0; i < m_apps.size(); ++i) {
        LVITEMW item = {0};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        
        // Convertir string a wstring para soporte Unicode
        std::wstring wstr(m_apps[i].name.begin(), m_apps[i].name.end());
        item.pszText = const_cast<LPWSTR>(wstr.c_str());
        ListView_InsertItem(m_hwndList, &item);
    }
}

void MainWindow::onSize(HWND hwnd, UINT flag, int width, int height) {
    if(!m_hwndList) return;
    
    const int padding = 10;
    const int buttonHeight = 30;
    const int buttonWidth = 120;
    
    // Resize list view
    SetWindowPos(m_hwndList, nullptr,
        padding, padding,
        width - 2*padding,
        height - 3*padding - buttonHeight,
        SWP_NOZORDER);
        
    // Reposition buttons
    int y = height - padding - buttonHeight;
    int x = padding;
    
    SetWindowPos(m_hwndBlockBtn, nullptr,
        x, y, buttonWidth, buttonHeight, SWP_NOZORDER);
    x += buttonWidth + padding;
    
    SetWindowPos(m_hwndAllowBtn, nullptr,
        x, y, buttonWidth, buttonHeight, SWP_NOZORDER);
    x += buttonWidth + padding;
    
    SetWindowPos(m_hwndExitBtn, nullptr,
        x, y, buttonWidth, buttonHeight, SWP_NOZORDER);
}

void MainWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    int sel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
    
    switch(id) {
        case ID_BLOCK:
        case ID_ALLOW: {
            if(sel == -1) {
                MessageBoxW(hwnd, L"Please select an application.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
            
            const ApplicationManager::Application& app = m_apps[sel];
            std::vector<std::string> executables;
            
            if(app.folderPath.empty()) {
                MessageBoxW(hwnd, L"Installation path not found for this application.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
            
            executables = ApplicationManager::getInstance().findExecutables(app.folderPath);
            
            if(executables.empty()) {
                MessageBoxW(hwnd, L"No executables found for this application.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
            
            std::string command;
            std::string ruleName = "Block_" + app.name;
            std::string exePath = executables[0]; // Use first executable found
            
            if(id == ID_BLOCK) {
                command = "netsh advfirewall firewall add rule name=\"" + ruleName + 
                         "\" dir=out action=block program=\"" + exePath + "\" enable=yes";
                
                if(system(command.c_str()) == 0) {
                    MessageBoxW(hwnd, L"Internet access blocked successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                } else {
                    MessageBoxW(hwnd, L"Failed to block internet access.", L"Error", MB_OK | MB_ICONERROR);
                }
            } else {
                command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";
                
                if(system(command.c_str()) == 0) {
                    MessageBoxW(hwnd, L"Internet access allowed successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                } else {
                    MessageBoxW(hwnd, L"Failed to allow internet access.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        
        case ID_EXIT:
            DestroyWindow(hwnd);
            break;
    }
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if(uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        
        pThis->createControls(hwnd);
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if(pThis) {
        switch(uMsg) {
            case WM_SIZE:
                pThis->onSize(hwnd, wParam, LOWORD(lParam), HIWORD(lParam));
                return 0;
            case WM_COMMAND:
                pThis->onCommand(hwnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ...rest of the implementation including event handlers...
