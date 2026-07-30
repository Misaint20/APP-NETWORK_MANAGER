#define UNICODE
#define _UNICODE
#include "MainWindow.h"
#include "../FirewallHelper.h"
#include "../StringHelper.h"
#include <windowsx.h>
#include <CommCtrl.h>
#include <thread>
#include <vector>
#include <cwctype>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Convenience alias
static LanguageManager& Lang() { return LanguageManager::getInstance(); }

// ── Helpers ──────────────────────────────────────────────────────────────

static std::wstring ToLower(const std::wstring& s) {
    std::wstring r = s;
    for (auto& c : r) c = (wchar_t)std::towlower(c);
    return r;
}

// ── Sort helpers for ListView column sorting ────────────────────────────

struct SortState {
    const std::vector<ApplicationManager::Application>* apps;
    const std::set<std::string>* blockedApps;
    int  column;
    bool ascending;
};

static int CALLBACK CompareApps(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    auto state = reinterpret_cast<const SortState*>(lParamSort);
    int idx1 = static_cast<int>(lParam1);
    int idx2 = static_cast<int>(lParam2);

    if (idx1 < 0 || idx1 >= (int)state->apps->size() ||
        idx2 < 0 || idx2 >= (int)state->apps->size())
        return 0;

    int result = 0;

    if (state->column == 0) {
        // Sort by application name (case-insensitive)
        const std::string& a = (*state->apps)[idx1].name;
        const std::string& b = (*state->apps)[idx2].name;
        result = _stricmp(a.c_str(), b.c_str());
    } else {
        // Sort by blocked status (blocked first), then by name
        std::string key1 = SanitizeForCmd((*state->apps)[idx1].name);
        std::string key2 = SanitizeForCmd((*state->apps)[idx2].name);
        bool blocked1 = (state->blockedApps->find(key1) != state->blockedApps->end());
        bool blocked2 = (state->blockedApps->find(key2) != state->blockedApps->end());

        if (blocked1 != blocked2)
            result = blocked1 ? -1 : 1;
        else
            result = _stricmp((*state->apps)[idx1].name.c_str(),
                              (*state->apps)[idx2].name.c_str());
    }

    return state->ascending ? result : -result;
}

// ── Singleton ────────────────────────────────────────────────────────────

MainWindow& MainWindow::getInstance() {
    static MainWindow instance;
    return instance;
}

// ── Window creation ──────────────────────────────────────────────────────

bool MainWindow::create(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"NetControlWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    if (!RegisterClassEx(&wc))
        return false;

    m_hwnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW | WS_EX_CONTROLPARENT,
        L"NetControlWindow",
        Lang().getString(StringId::WindowTitle).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 580, 480,
        nullptr, nullptr, hInstance, this);

    return (m_hwnd != nullptr);
}

// ── Show window ──────────────────────────────────────────────────────────

void MainWindow::show(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
}

// ── Control creation ─────────────────────────────────────────────────────

void MainWindow::createControls(HWND hwnd) {
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

    // Common controls
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // Fonts  —──────────────────────────────────────────────────────────────
    m_hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hBoldFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    /// 1. Search edit  ─────────────────────────────────────────────────────
    m_hwndSearchEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd, (HMENU)ID_SEARCH_EDIT, hInst, nullptr);

    /// 1b. Refresh button  ─────────────────────────────────────────────────
    m_hwndRefreshBtn = CreateWindowEx(
        0, L"BUTTON", Lang().getString(StringId::Refresh).c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER,
        0, 0, 0, 0, hwnd, (HMENU)ID_REFRESH, hInst, nullptr);

    /// 1c. Language combo box  ──────────────────────────────────────────────
    m_hwndLangCombo = CreateWindowEx(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        0, 0, 0, 0, hwnd, (HMENU)ID_LANG_COMBO, hInst, nullptr);

    for(int i = 0; i < static_cast<int>(Language::Count); ++i)
        SendMessageW(m_hwndLangCombo, CB_ADDSTRING, 0,
                     (LPARAM)LanguageManager::getLanguageName(static_cast<Language>(i)));
    SendMessageW(m_hwndLangCombo, CB_SETCURSEL,
                 static_cast<int>(Lang().getLanguage()), 0);

    /// 2. Header label  ────────────────────────────────────────────────────
    m_hwndHeaderLabel = CreateWindowEx(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, nullptr, hInst, nullptr);

    /// 3. Working indicator  ───────────────────────────────────────────────
    m_hwndWorkingLabel = CreateWindowEx(
        0, L"STATIC", Lang().getString(StringId::Working).c_str(),
        WS_CHILD,
        0, 0, 0, 0, hwnd, nullptr, hInst, nullptr);

    /// 4. ListView  ────────────────────────────────────────────────────────
    m_hwndList = CreateWindowEx(
        0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hwnd, (HMENU)ID_LISTVIEW, hInst, nullptr);

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    std::wstring s0 = Lang().getString(StringId::ColumnAppName);
    lvc.cx = 460;  lvc.pszText = const_cast<LPWSTR>(s0.c_str());
    ListView_InsertColumn(m_hwndList, 0, &lvc);

    std::wstring s1 = Lang().getString(StringId::ColumnStatus);
    lvc.cx = 80;   lvc.pszText = const_cast<LPWSTR>(s1.c_str());
    ListView_InsertColumn(m_hwndList, 1, &lvc);

    ListView_SetExtendedListViewStyle(m_hwndList,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

    /// 5. Action buttons  ──────────────────────────────────────────────────
    const DWORD btnSt = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER;

    m_hwndBlockBtn = CreateWindowEx(0, L"BUTTON",
        Lang().getString(StringId::BtnBlock).c_str(),
        btnSt, 0,0,0,0, hwnd, (HMENU)ID_BLOCK, hInst, nullptr);

    m_hwndAllowBtn = CreateWindowEx(0, L"BUTTON",
        Lang().getString(StringId::BtnAllow).c_str(),
        btnSt, 0,0,0,0, hwnd, (HMENU)ID_ALLOW, hInst, nullptr);

    m_hwndExitBtn = CreateWindowEx(0, L"BUTTON",
        Lang().getString(StringId::BtnExit).c_str(),
        btnSt, 0,0,0,0, hwnd, (HMENU)ID_EXIT, hInst, nullptr);

    m_hwndUnblockAllBtn = CreateWindowEx(0, L"BUTTON",
        Lang().getString(StringId::BtnUnblockAll).c_str(),
        btnSt, 0,0,0,0, hwnd, (HMENU)ID_UNBLOCK_ALL, hInst, nullptr);

    /// 6. Status bar  ──────────────────────────────────────────────────────
    m_hwndStatusBar = CreateWindowEx(
        0, L"STATIC", Lang().getString(StringId::Scanning).c_str(),
        WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
        0,0,0,0, hwnd, nullptr, hInst, nullptr);

    // Apply fonts
    SendMessage(m_hwndLangCombo,     WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndSearchEdit,    WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndRefreshBtn,    WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndHeaderLabel,   WM_SETFONT, (WPARAM)m_hBoldFont, TRUE);
    SendMessage(m_hwndWorkingLabel,  WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndBlockBtn,      WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndAllowBtn,      WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndExitBtn,       WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndUnblockAllBtn, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndList,          WM_SETFONT, (WPARAM)m_hFont, TRUE);
    // Set ListView header font separately
    HWND hHeader = ListView_GetHeader(m_hwndList);
    if (hHeader) SendMessage(hHeader, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessage(m_hwndStatusBar,     WM_SETFONT, (WPARAM)m_hFont, TRUE);    ShowWindow(m_hwndWorkingLabel, SW_HIDE);

    /// 7. Tooltips for action buttons  ─────────────────────────────────────
    m_hwndTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr,
        TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwnd, nullptr, hInst, nullptr);

    if (m_hwndTooltip) {
        SendMessageW(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 300);

        auto addTool = [&](int ctrlId) {
            TOOLINFOW ti = { sizeof(ti), TTF_SUBCLASS };
            ti.hwnd = hwnd;
            ti.uId = static_cast<UINT_PTR>(ctrlId);
            ti.lpszText = LPSTR_TEXTCALLBACKW;
            SendMessageW(m_hwndTooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        };

        addTool(ID_BLOCK);
        addTool(ID_ALLOW);
        addTool(ID_UNBLOCK_ALL);
        addTool(ID_REFRESH);
        addTool(ID_EXIT);
    }

    // Populate
    setStatusText(Lang().getString(StringId::ScanningFirewall));
    loadColumnState();  // Restore saved column widths/order/sort
    refreshApplicationList();
    updateStatusSummary();
}

// ── UI string refresher (called when language changes) ───────────────────

void MainWindow::refreshUIStrings() {
    // Window title
    SetWindowTextW(m_hwnd, Lang().getString(StringId::WindowTitle).c_str());

    // Buttons & labels
    SetWindowTextW(m_hwndRefreshBtn,
        Lang().getString(StringId::Refresh).c_str());
    SetWindowTextW(m_hwndWorkingLabel, Lang().getString(StringId::Working).c_str());
    SetWindowTextW(m_hwndBlockBtn,      Lang().getString(StringId::BtnBlock).c_str());
    SetWindowTextW(m_hwndAllowBtn,      Lang().getString(StringId::BtnAllow).c_str());
    SetWindowTextW(m_hwndExitBtn,       Lang().getString(StringId::BtnExit).c_str());
    SetWindowTextW(m_hwndUnblockAllBtn, Lang().getString(StringId::BtnUnblockAll).c_str());

    // Column headers
    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT;
    std::wstring cn = Lang().getString(StringId::ColumnAppName);
    lvc.pszText = const_cast<LPWSTR>(cn.c_str());
    ListView_SetColumn(m_hwndList, 0, &lvc);
    std::wstring cs = Lang().getString(StringId::ColumnStatus);
    lvc.pszText = const_cast<LPWSTR>(cs.c_str());
    ListView_SetColumn(m_hwndList, 1, &lvc);

    updateHeaderLabel();
    updateStatusColumn();
    updateStatusSummary();
}

void MainWindow::onLanguageChanged() {
    int sel = (int)SendMessageW(m_hwndLangCombo, CB_GETCURSEL, 0, 0);
    if(sel >= 0 && sel < static_cast<int>(Language::Count)) {
        Lang().setLanguage(static_cast<Language>(sel));
        refreshUIStrings();
    }
}

// ── List helpers ─────────────────────────────────────────────────────────

void MainWindow::updateStatusColumn() {
    for(int i = 0; i < ListView_GetItemCount(m_hwndList); ++i) {
        LVITEMW lvi = {0};
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = i;
        ListView_GetItem(m_hwndList, &lvi);
        int appIdx = static_cast<int>(lvi.lParam);

        if(appIdx >= 0 && appIdx < (int)m_apps.size()) {
            std::string key = SanitizeForCmd(m_apps[appIdx].name);
            bool blocked = (m_blockedApps.find(key) != m_blockedApps.end());
            const std::wstring& txt = blocked
                ? Lang().getString(StringId::StatusBlocked)
                : Lang().getString(StringId::StatusAllowed);
            ListView_SetItemText(m_hwndList, i, 1, const_cast<LPWSTR>(txt.c_str()));
        }
    }
}

void MainWindow::populateBlockedStatus() {
    m_blockedApps = ApplicationManager::getBlockedAppNames();
    computeOrphanedApps();
    updateStatusColumn();
}

void MainWindow::computeOrphanedApps() {
    m_orphanedApps.clear();
    for (const auto& blocked : m_blockedApps) {
        bool found = false;
        for (const auto& app : m_apps) {
            if (SanitizeForCmd(app.name) == blocked) {
                found = true;
                break;
            }
        }
        if (!found)
            m_orphanedApps.insert(blocked);
    }
}

void MainWindow::refreshApplicationList() {
    // Batch updates: prevent flickering during bulk insert
    SendMessageW(m_hwndList, WM_SETREDRAW, FALSE, 0);

    ListView_DeleteAllItems(m_hwndList);
    m_apps = ApplicationManager::getInstance().getInstalledApplications();

    for (size_t i = 0; i < m_apps.size(); ++i) {
        std::wstring name(m_apps[i].name.begin(), m_apps[i].name.end());

        LVITEMW item = {0};
        item.mask    = LVIF_TEXT | LVIF_PARAM;
        item.iItem   = static_cast<int>(i);
        item.pszText = const_cast<LPWSTR>(name.c_str());
        item.lParam  = (LPARAM)i;
        ListView_InsertItem(m_hwndList, &item);
    }

    populateBlockedStatus();
    applySort();
    updateHeaderLabel();

    // Re-enable drawing
    SendMessageW(m_hwndList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(m_hwndList, nullptr, TRUE);
}

void MainWindow::applyFilter() {
    WCHAR buf[256];
    GetWindowTextW(m_hwndSearchEdit, buf, 256);
    CharLowerW(buf);
    std::wstring filter(buf);

    // Batch updates: prevent flickering during bulk insert
    SendMessageW(m_hwndList, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(m_hwndList);

    int vis = 0;
    for (size_t i = 0; i < m_apps.size(); ++i) {
        std::wstring name(m_apps[i].name.begin(), m_apps[i].name.end());
        std::wstring lower = ToLower(name);

        if (filter.empty() || lower.find(filter) != std::wstring::npos) {
            LVITEMW item = {0};
            item.mask    = LVIF_TEXT | LVIF_PARAM;
            item.iItem   = vis++;
            item.pszText = const_cast<LPWSTR>(name.c_str());
            item.lParam  = (LPARAM)i;
            ListView_InsertItem(m_hwndList, &item);
        }
    }

    updateStatusColumn();
    applySort();
    updateHeaderLabel();

    // Re-enable drawing
    SendMessageW(m_hwndList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(m_hwndList, nullptr, TRUE);
}

void MainWindow::updateHeaderLabel() {
    int shown = ListView_GetItemCount(m_hwndList);

    // Check if search filter is active with no results
    bool filterActive = (GetWindowTextLengthW(m_hwndSearchEdit) > 0);

    if (filterActive && shown == 0) {
        SetWindowTextW(m_hwndHeaderLabel,
            Lang().getString(StringId::EmptyFilterText).c_str());
    } else {
        SetWindowTextW(m_hwndHeaderLabel,
            Lang().getStringF(StringId::HeaderLabel,
                std::to_wstring(shown), std::to_wstring(m_apps.size())).c_str());
    }
}

int MainWindow::getSelectedAppIndex() {
    int item = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
    if (item == -1) return -1;

    LVITEMW lvi = {0};
    lvi.mask  = LVIF_PARAM;
    lvi.iItem = item;
    ListView_GetItem(m_hwndList, &lvi);
    return static_cast<int>(lvi.lParam);
}

void MainWindow::setStatusText(const std::wstring& text) {
    SetWindowTextW(m_hwndStatusBar, text.c_str());
}

void MainWindow::updateStatusSummary() {
    size_t blocked  = m_blockedApps.size();
    size_t total    = m_apps.size();
    size_t orphaned = m_orphanedApps.size();

    // Build sort indicator: "Name \u25B2" or "Status \u25BC"
    std::wstring sortInfo;
    if (m_sortColumn >= 0) {
        sortInfo = (m_sortColumn == 0)
            ? Lang().getString(StringId::ColumnAppName)
            : Lang().getString(StringId::ColumnStatus);
        sortInfo += m_sortAscending ? L" \u25B2" : L" \u25BC";
    }

    std::wstring text = sortInfo;
    if (!text.empty()) text += L"  |  ";
    text += Lang().getStringF(StringId::BlockedSummary,
        std::to_wstring(blocked), std::to_wstring(total));
    if (orphaned > 0)
        text += L"  |  " + std::to_wstring(orphaned) + L" orphaned";
    setStatusText(text);
}

// ── Layout ───────────────────────────────────────────────────────────────

void MainWindow::onSize(HWND hwnd, UINT flag, int width, int height) {
    if (!m_hwndList) return;

    const int PAD       = 10;
    const int SEARCH_H  = 24;
    const int HEADER_H  = 20;
    const int BTN_H     = 30;
    const int STATUS_H  = 24;
    const int REFRESH_W = 70;
    const int LANG_W    = 120;

    // 1. Top row: Search edit + Refresh button + Language combo
    int elemW = width - 2 * PAD;                     // usable width
    int searchW = elemW - REFRESH_W - LANG_W - 2 * PAD;
    if (searchW < 50) searchW = 50;

    int x = PAD;
    SetWindowPos(m_hwndSearchEdit, nullptr, x, PAD, searchW, SEARCH_H, SWP_NOZORDER);
    x += searchW + PAD;
    SetWindowPos(m_hwndRefreshBtn, nullptr, x, PAD, REFRESH_W, SEARCH_H, SWP_NOZORDER);
    x += REFRESH_W + PAD;
    SetWindowPos(m_hwndLangCombo,  nullptr, x, PAD, LANG_W,   200, SWP_NOZORDER);
    // 200 = enough height to show the dropdown list (15 language items)

    // 2. Header label
    int hdrY = PAD + SEARCH_H + 4;
    SetWindowPos(m_hwndHeaderLabel, nullptr, PAD, hdrY,
                 width - 2 * PAD, HEADER_H, SWP_NOZORDER);

    // 3. Working indicator (right side of header row)
    SetWindowPos(m_hwndWorkingLabel, nullptr,
                 width - PAD - 130, hdrY, 130, HEADER_H, SWP_NOZORDER);

    // 4. ListView (fills middle)
    int listY = hdrY + HEADER_H + 4;
    int listH = height - listY - PAD - BTN_H - 4 - STATUS_H;
    if (listH < 30) listH = 30;
    SetWindowPos(m_hwndList, nullptr, PAD, listY,
                 width - 2 * PAD, listH, SWP_NOZORDER);

    // Columns — only auto-size if user hasn't manually dragged a divider
    if (!m_columnsUserSet) {
        const int STATUS_W = 80;
        ListView_SetColumnWidth(m_hwndList, 0, width - 2 * PAD - 6 - STATUS_W);
        ListView_SetColumnWidth(m_hwndList, 1, STATUS_W);
    }

    // 5. Buttons — centered (4 buttons)
    int butY = listY + listH + PAD;
    const int BTN4_W = 110;
    int totalBtnW = 4 * BTN4_W + 3 * PAD;
    int btnStartX = (width - totalBtnW) / 2;

    SetWindowPos(m_hwndBlockBtn,      nullptr, btnStartX,                           butY, BTN4_W, BTN_H, SWP_NOZORDER);
    SetWindowPos(m_hwndAllowBtn,      nullptr, btnStartX + BTN4_W + PAD,             butY, BTN4_W, BTN_H, SWP_NOZORDER);
    SetWindowPos(m_hwndUnblockAllBtn, nullptr, btnStartX + 2*(BTN4_W + PAD),         butY, BTN4_W, BTN_H, SWP_NOZORDER);
    SetWindowPos(m_hwndExitBtn,       nullptr, btnStartX + 3*(BTN4_W + PAD),         butY, BTN4_W, BTN_H, SWP_NOZORDER);

    // 6. Status bar at very bottom
    SetWindowPos(m_hwndStatusBar, nullptr, 0, height - STATUS_H,
                 width, STATUS_H, SWP_NOZORDER);
}

// ── Firewall operation ───────────────────────────────────────────────────

void MainWindow::startFirewallOperation(HWND hwnd, int id) {
    // Atomic guard: prevent concurrent firewall operations
    bool expected = false;
    if (!m_firewallRunning.compare_exchange_strong(expected, true)) {
        MessageBoxW(hwnd, L"A firewall operation is already in progress.",
                    L"Please wait", MB_OK | MB_ICONINFORMATION);
        return;
    }

    int idx = getSelectedAppIndex();
    if (idx < 0 || idx >= (int)m_apps.size()) {
        m_firewallRunning.store(false);
        MessageBoxW(hwnd,
            Lang().getString(StringId::SelectAppFirst).c_str(),
            Lang().getString(StringId::InfoTitle).c_str(),
            MB_OK | MB_ICONINFORMATION);
        return;
    }

    const auto& app = m_apps[idx];

    if (app.folderPath.empty()) {
        m_firewallRunning.store(false);
        MessageBoxW(hwnd,
            Lang().getString(StringId::InstallPathNotFound).c_str(),
            Lang().getString(StringId::ErrorTitle).c_str(),
            MB_OK | MB_ICONERROR);
        return;
    }

    std::vector<std::string> executables =
        ApplicationManager::getInstance().findExecutables(app.folderPath);

    if (executables.empty()) {
        m_firewallRunning.store(false);
        MessageBoxW(hwnd,
            Lang().getString(StringId::NoExecutablesFound).c_str(),
            Lang().getString(StringId::ErrorTitle).c_str(),
            MB_OK | MB_ICONERROR);
        return;
    }

    // Build command (rule names are always English — Windows Firewall internal)
    std::string safeName = SanitizeForCmd(app.name);
    std::string safeExe  = SanitizeForCmd(executables[0]);
    std::string ruleName = "Block_" + safeName;
    m_lastFirewallKey = safeName;  // Save for direct status update in onFirewallComplete

    std::string command;
    if (id == ID_BLOCK)
        command = "netsh advfirewall firewall add rule name=\"" + ruleName +
                  "\" dir=out action=block program=\"" + safeExe + "\" enable=yes";
    else
        command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";

    // UI feedback
    EnableWindow(m_hwndBlockBtn,      FALSE);
    EnableWindow(m_hwndAllowBtn,      FALSE);
    EnableWindow(m_hwndUnblockAllBtn, FALSE);
    EnableWindow(m_hwndRefreshBtn,    FALSE);
    EnableWindow(m_hwndLangCombo,     FALSE);
    ShowWindow(m_hwndWorkingLabel, SW_SHOW);

    std::wstring appWide(app.name.begin(), app.name.end());
    setStatusText(id == ID_BLOCK
        ? Lang().getStringF(StringId::BlockingAccess, appWide)
        : Lang().getStringF(StringId::AllowingAccess, appWide));

    // Async execution via shared helper
    std::thread([hwnd, command, isBlock = (id == ID_BLOCK)]() {
        int rc = ExecuteFirewallCommand(command);
        DWORD exitCode = (rc >= 0) ? (DWORD)rc : 1;
        PostMessageW(hwnd, WM_FIREWALL_COMPLETE, (WPARAM)exitCode, (LPARAM)isBlock);
    }).detach();
}

// ── Command handler ──────────────────────────────────────────────────────

void MainWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (id) {

    case ID_BLOCK:
    case ID_ALLOW:
        startFirewallOperation(hwnd, id);
        break;

    case ID_UNBLOCK_ALL:
        startUnblockAllOperation(hwnd);
        break;

    case ID_CLEANUP_ORPHANS:
        startCleanupOrphans(hwnd);
        break;

    case ID_RESET_COLUMNS:
        m_columnsUserSet = false;
        // Trigger onSize to recalculate column widths for current window size
        RECT rc;
        GetClientRect(hwnd, &rc);
        onSize(hwnd, 0, rc.right - rc.left, rc.bottom - rc.top);
        break;

    case ID_EXIT:
        DestroyWindow(hwnd);
        break;

    case ID_REFRESH: {
        setStatusText(Lang().getString(StringId::Scanning));
        EnableWindow(m_hwndRefreshBtn, FALSE);
        refreshApplicationList();
        SetWindowTextW(m_hwndSearchEdit, L"");
        EnableWindow(m_hwndRefreshBtn, TRUE);
        updateStatusSummary();
        break;
    }

    default:
        if (id == ID_SEARCH_EDIT && codeNotify == EN_CHANGE)
            applyFilter();
        else if (id == ID_LANG_COMBO && codeNotify == CBN_SELCHANGE)
            onLanguageChanged();
        break;
    }
}

// ── Column state persistence (Registry) ──────────────────────────────

static const wchar_t* REGKEY = L"Software\\APP-NETWORK_MANAGER";

void MainWindow::saveColumnState() {
    if (!m_hwndList) return;
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REGKEY, 0, nullptr,
                        0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return;

    // Current column widths (by visual order)
    DWORD w0 = ListView_GetColumnWidth(m_hwndList, 0);
    DWORD w1 = ListView_GetColumnWidth(m_hwndList, 1);
    RegSetValueExW(hKey, L"ColWidth0", 0, REG_DWORD, (BYTE*)&w0, sizeof(w0));
    RegSetValueExW(hKey, L"ColWidth1", 0, REG_DWORD, (BYTE*)&w1, sizeof(w1));

    // Column order array
    DWORD order[2];
    ListView_GetColumnOrderArray(m_hwndList, 2, order);
    RegSetValueExW(hKey, L"ColOrder0", 0, REG_DWORD, (BYTE*)&order[0], sizeof(DWORD));
    RegSetValueExW(hKey, L"ColOrder1", 0, REG_DWORD, (BYTE*)&order[1], sizeof(DWORD));

    // Sort state
    DWORD sCol = static_cast<DWORD>(m_sortColumn >= 0 ? m_sortColumn : 0);
    DWORD sAsc = m_sortAscending ? 1 : 0;
    DWORD usr = m_columnsUserSet ? 1 : 0;
    RegSetValueExW(hKey, L"SortCol",      0, REG_DWORD, (BYTE*)&sCol, sizeof(sCol));
    RegSetValueExW(hKey, L"SortAsc",      0, REG_DWORD, (BYTE*)&sAsc, sizeof(sAsc));
    RegSetValueExW(hKey, L"ColsUserSet",  0, REG_DWORD, (BYTE*)&usr,  sizeof(usr));

    RegCloseKey(hKey);
}

void MainWindow::loadColumnState() {
    if (!m_hwndList) return;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGKEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

    // Read widths
    DWORD w0 = 460, w1 = 80, type, size = sizeof(DWORD);
    RegQueryValueExW(hKey, L"ColWidth0", nullptr, &type, (BYTE*)&w0, &size);
    RegQueryValueExW(hKey, L"ColWidth1", nullptr, &type, (BYTE*)&w1, &size);

    // Read column order
    DWORD order[2] = {0, 1};
    size = sizeof(DWORD);
    RegQueryValueExW(hKey, L"ColOrder0", nullptr, &type, (BYTE*)&order[0], &size);
    RegQueryValueExW(hKey, L"ColOrder1", nullptr, &type, (BYTE*)&order[1], &size);

    // Read sort state
    DWORD sCol = 0, sAsc = 1, usr = 0;
    size = sizeof(DWORD);
    RegQueryValueExW(hKey, L"SortCol",     nullptr, &type, (BYTE*)&sCol, &size);
    size = sizeof(DWORD);
    RegQueryValueExW(hKey, L"SortAsc",     nullptr, &type, (BYTE*)&sAsc, &size);
    size = sizeof(DWORD);
    RegQueryValueExW(hKey, L"ColsUserSet", nullptr, &type, (BYTE*)&usr,  &size);

    RegCloseKey(hKey);

    // Apply column widths
    ListView_SetColumnWidth(m_hwndList, 0, w0);
    ListView_SetColumnWidth(m_hwndList, 1, w1);

    // Apply column order (reorder columns if needed)
    if (order[0] != 0 || order[1] != 1)
        ListView_SetColumnOrderArray(m_hwndList, 2, order);

    // Restore sort & column user-set state
    m_sortColumn     = static_cast<int>(sCol);
    m_sortAscending  = (sAsc != 0);
    m_columnsUserSet = (usr != 0);
}

// ── Sort / Notify handlers ──────────────────────────────────────────────

void MainWindow::onColumnClick(int column) {
    if (column == m_sortColumn)
        m_sortAscending = !m_sortAscending;
    else {
        m_sortColumn    = column;
        m_sortAscending = true;
    }
    applySort();
}

void MainWindow::applySort() {
    if (m_sortColumn < 0 || !m_hwndList) return;

    SortState state = { &m_apps, &m_blockedApps, m_sortColumn, m_sortAscending };
    ListView_SortItems(m_hwndList, CompareApps, reinterpret_cast<LPARAM>(&state));

    // Update sort arrows on header
    HWND hHeader = ListView_GetHeader(m_hwndList);
    if (!hHeader) return;
    for (int i = 0; i < 2; ++i) {
        HDITEM hdi = {0};
        hdi.mask = HDI_FORMAT;
        Header_GetItem(hHeader, i, &hdi);
        UINT arrow = (i == m_sortColumn)
            ? (m_sortAscending ? HDF_SORTUP : HDF_SORTDOWN)
            : 0;
        hdi.fmt = (hdi.fmt & ~(HDF_SORTUP | HDF_SORTDOWN)) | arrow;
        Header_SetItem(hHeader, i, &hdi);
    }
    updateStatusSummary();
}

void MainWindow::onNotify(HWND hwnd, LPNMHDR nmhdr) {
    // Track user column resize (header divider drag) or column reorder
    if (nmhdr->code == HDN_ENDTRACK || nmhdr->code == HDN_ENDDRAG) {
        m_columnsUserSet = true;
        return;
    }

    // Tooltip text for action buttons (TTN_GETDISPINFO)
    if (nmhdr->code == TTN_GETDISPINFO) {
        auto pTdi = reinterpret_cast<NMTTDISPINFOW*>(nmhdr);
        UINT id = static_cast<UINT>(pTdi->hdr.idFrom);
        switch (id) {
            case ID_BLOCK:
                wcsncpy(m_tooltipBuffer,
                    Lang().getString(StringId::BtnBlock).c_str(), 255);
                break;
            case ID_ALLOW:
                wcsncpy(m_tooltipBuffer,
                    Lang().getString(StringId::BtnAllow).c_str(), 255);
                break;
            case ID_UNBLOCK_ALL:
                wcsncpy(m_tooltipBuffer,
                    Lang().getString(StringId::BtnUnblockAll).c_str(), 255);
                break;
            case ID_REFRESH:
                wcsncpy(m_tooltipBuffer,
                    Lang().getString(StringId::Refresh).c_str(), 255);
                break;
            case ID_EXIT:
                wcsncpy(m_tooltipBuffer,
                    Lang().getString(StringId::BtnExit).c_str(), 255);
                break;
            default:
                return;
        }
        m_tooltipBuffer[255] = L'\0';
        pTdi->lpszText = m_tooltipBuffer;
        return;
    }

    if (nmhdr->idFrom != ID_LISTVIEW) return;

    switch (nmhdr->code) {
    case NM_DBLCLK:
        startFirewallOperation(hwnd, ID_BLOCK);
        break;
    case LVN_COLUMNCLICK: {
        auto pnmv = reinterpret_cast<NMLISTVIEW*>(nmhdr);
        onColumnClick(pnmv->iSubItem);
        break;
    }
    case LVN_GETINFOTIP: {
        auto pInfo = reinterpret_cast<NMLVGETINFOTIPW*>(nmhdr);
        int appIdx = static_cast<int>(pInfo->lParam);
        if (appIdx >= 0 && appIdx < (int)m_apps.size()) {
            const auto& app = m_apps[appIdx];
            std::string key = SanitizeForCmd(app.name);
            bool blocked = (m_blockedApps.find(key) != m_blockedApps.end());

            std::wstring appName(app.name.begin(), app.name.end());
            std::wstring folder(app.folderPath.begin(), app.folderPath.end());

            // Build tip: Status + Name
            std::wstring tip;
            if (blocked) {
                tip = L"\U0001F6AB " + appName + L" \u2014 " +
                      Lang().getString(StringId::StatusBlocked);
            } else {
                tip = L"\u2705 " + appName + L" \u2014 " +
                      Lang().getString(StringId::StatusAllowed);
            }

            // Add folder path on a new line
            if (!folder.empty()) {
                tip += L"\n\U0001F4C1 " + folder;
            }

            // For blocked apps: show firewall rule name
            if (blocked) {
                std::string ruleName = "Block_" + key;
                std::wstring ruleWide(ruleName.begin(), ruleName.end());
                tip += L"\n\U0001F6E1 \u2022 " + ruleWide;
            }

            wcsncpy(pInfo->pszText, tip.c_str(), pInfo->cchTextMax - 1);
            pInfo->pszText[pInfo->cchTextMax - 1] = L'\0';
        }
        break;
    }
    }
}

// ── Unblock All operation ───────────────────────────────────────────────

void MainWindow::startUnblockAllOperation(HWND hwnd) {
    if (m_blockedApps.empty()) {
        MessageBoxW(hwnd, L"No blocked applications to unblock.",
                    L"Information", MB_OK | MB_ICONINFORMATION);
        return;
    }

    bool expected = false;
    if (!m_firewallRunning.compare_exchange_strong(expected, true)) {
        MessageBoxW(hwnd, L"A firewall operation is already in progress.",
                    L"Please wait", MB_OK | MB_ICONINFORMATION);
        return;
    }

    // Disable all action buttons
    EnableWindow(m_hwndBlockBtn,      FALSE);
    EnableWindow(m_hwndAllowBtn,      FALSE);
    EnableWindow(m_hwndUnblockAllBtn, FALSE);
    EnableWindow(m_hwndRefreshBtn,    FALSE);
    EnableWindow(m_hwndLangCombo,     FALSE);
    ShowWindow(m_hwndWorkingLabel, SW_SHOW);
    setStatusText(L"Unblocking all applications...");

    // Copy blocked set so the thread owns its data
    auto blocked = m_blockedApps;
    std::thread([hwnd, blocked]() {
        int succeeded = 0, failed = 0;
        for (const auto& key : blocked) {
            std::string cmd = "netsh advfirewall firewall delete rule name=\"Block_"
                            + key + "\"";
            int rc = ExecuteFirewallCommand(cmd);
            if (rc == 0) ++succeeded; else ++failed;
        }
        PostMessageW(hwnd, WM_UNBLOCK_ALL_COMPLETE,
                     (WPARAM)succeeded, (LPARAM)failed);
    }).detach();
}

void MainWindow::onUnblockAllComplete(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    int succeeded = static_cast<int>(wParam);
    int failed    = static_cast<int>(lParam);

    EnableWindow(m_hwndBlockBtn,      TRUE);
    EnableWindow(m_hwndAllowBtn,      TRUE);
    EnableWindow(m_hwndUnblockAllBtn, TRUE);
    EnableWindow(m_hwndRefreshBtn,    TRUE);
    EnableWindow(m_hwndLangCombo,     TRUE);
    ShowWindow(m_hwndWorkingLabel, SW_HIDE);

    m_firewallRunning.store(false);
    populateBlockedStatus();

    std::wstring msg = L"Unblocked " + std::to_wstring(succeeded) + L" application(s).";
    if (failed > 0)
        msg += L"\n" + std::to_wstring(failed) + L" failed.";

    MessageBoxW(hwnd, msg.c_str(), L"Unblock All Complete", MB_OK | MB_ICONINFORMATION);
    updateStatusSummary();
}

// ── Cleanup Orphans operation ───────────────────────────────────────────

void MainWindow::startCleanupOrphans(HWND hwnd) {
    if (m_orphanedApps.empty()) {
        MessageBoxW(hwnd, L"No orphaned rules to clean up.",
                    L"Information", MB_OK | MB_ICONINFORMATION);
        return;
    }

    bool expected = false;
    if (!m_firewallRunning.compare_exchange_strong(expected, true)) {
        MessageBoxW(hwnd, L"A firewall operation is already in progress.",
                    L"Please wait", MB_OK | MB_ICONINFORMATION);
        return;
    }

    EnableWindow(m_hwndBlockBtn,      FALSE);
    EnableWindow(m_hwndAllowBtn,      FALSE);
    EnableWindow(m_hwndUnblockAllBtn, FALSE);
    EnableWindow(m_hwndRefreshBtn,    FALSE);
    EnableWindow(m_hwndLangCombo,     FALSE);
    ShowWindow(m_hwndWorkingLabel, SW_SHOW);
    setStatusText(L"Cleaning up orphaned rules...");

    auto orphans = m_orphanedApps;  // copy
    std::thread([hwnd, orphans]() {
        int succeeded = 0, failed = 0;
        for (const auto& key : orphans) {
            std::string cmd = "netsh advfirewall firewall delete rule name=\"Block_"
                            + key + "\"";
            int rc = ExecuteFirewallCommand(cmd);
            if (rc == 0) ++succeeded; else ++failed;
        }
        PostMessageW(hwnd, WM_CLEANUP_ORPHANS_COMPLETE,
                     (WPARAM)succeeded, (LPARAM)failed);
    }).detach();
}

void MainWindow::onCleanupOrphansComplete(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    EnableWindow(m_hwndBlockBtn,      TRUE);
    EnableWindow(m_hwndAllowBtn,      TRUE);
    EnableWindow(m_hwndUnblockAllBtn, TRUE);
    EnableWindow(m_hwndRefreshBtn,    TRUE);
    EnableWindow(m_hwndLangCombo,     TRUE);
    ShowWindow(m_hwndWorkingLabel, SW_HIDE);

    m_firewallRunning.store(false);
    populateBlockedStatus();

    std::wstring msg = L"Removed " + std::to_wstring(static_cast<int>(wParam))
                     + L" orphaned rule(s).";
    if (static_cast<int>(lParam) > 0)
        msg += L"\n" + std::to_wstring(static_cast<int>(lParam)) + L" failed.";
    MessageBoxW(hwnd, msg.c_str(), L"Cleanup Complete", MB_OK | MB_ICONINFORMATION);
    updateStatusSummary();
}

// ── Context menu (right‑click on ListView) ─────────────────────────────

void MainWindow::onContextMenu(HWND hwnd, int screenX, int screenY) {
    // Convert to client coords relative to ListView
    POINT pt = { screenX, screenY };
    ScreenToClient(m_hwndList, &pt);

    // Hit-test to find the item under the cursor
    LVHITTESTINFO ht = {0};
    ht.pt = pt;
    int item = ListView_HitTest(m_hwndList, &ht);
    if (item < 0)
        return;

    // Select the item under cursor (don't require prior left-click)
    ListView_SetItemState(m_hwndList, item,
        LVIS_SELECTED | LVIS_FOCUSED,
        LVIS_SELECTED | LVIS_FOCUSED);

    // Build popup menu
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    AppendMenuW(hMenu, MF_STRING, ID_BLOCK,
        Lang().getString(StringId::BtnBlock).c_str());
    AppendMenuW(hMenu, MF_STRING, ID_ALLOW,
        Lang().getString(StringId::BtnAllow).c_str());
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_UNBLOCK_ALL,
        Lang().getString(StringId::BtnUnblockAll).c_str());

    // Add Cleanup Orphans if any exist
    if (!m_orphanedApps.empty()) {
        std::wstring orphanLabel = L"Cleanup " + std::to_wstring(m_orphanedApps.size()) + L" Orphaned Rule(s)";
        AppendMenuW(hMenu, MF_STRING, ID_CLEANUP_ORPHANS, orphanLabel.c_str());
    }

    // Add Reset Columns option (last)
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_RESET_COLUMNS, L"Reset Columns Width");

    // Show the context menu
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                   screenX, screenY, 0, hwnd, nullptr);
    PostMessageW(hwnd, WM_NULL, 0, 0);  // workaround for menu re-entry

    DestroyMenu(hMenu);
}

// ── Firewall completion callback ─────────────────────────────────────────

void MainWindow::onFirewallComplete(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    DWORD exitCode = (DWORD)wParam;
    bool  wasBlock = (lParam != 0);

    EnableWindow(m_hwndBlockBtn,      TRUE);
    EnableWindow(m_hwndAllowBtn,      TRUE);
    EnableWindow(m_hwndUnblockAllBtn, TRUE);
    EnableWindow(m_hwndRefreshBtn,    TRUE);
    EnableWindow(m_hwndLangCombo,     TRUE);
    ShowWindow(m_hwndWorkingLabel, SW_HIDE);

    m_firewallRunning.store(false);

    // Directly update blocked status (no netsh parsing needed — locale-independent)
    if (exitCode == 0 && !m_lastFirewallKey.empty()) {
        if (wasBlock)
            m_blockedApps.insert(m_lastFirewallKey);
        else
            m_blockedApps.erase(m_lastFirewallKey);
    }
    computeOrphanedApps();
    updateStatusColumn();
    applySort();
    InvalidateRect(m_hwndList, nullptr, TRUE);

    // Also refresh from netsh in background to catch any external changes
    if (exitCode == 0) {
        setStatusText(wasBlock
            ? Lang().getString(StringId::AccessBlocked)
            : Lang().getString(StringId::AccessAllowed));
        MessageBoxW(hwnd,
            (wasBlock ? Lang().getString(StringId::SuccessBlockMsg)
                      : Lang().getString(StringId::SuccessAllowMsg)).c_str(),
            Lang().getString(StringId::SuccessTitle).c_str(),
            MB_OK | MB_ICONINFORMATION);
    } else {
        setStatusText(wasBlock
            ? Lang().getString(StringId::AccessBlockFailed)
            : Lang().getString(StringId::AccessAllowFailed));
        MessageBoxW(hwnd,
            (wasBlock ? Lang().getString(StringId::FailBlockMsg)
                      : Lang().getString(StringId::FailAllowMsg)).c_str(),
            Lang().getString(StringId::ErrorTitle).c_str(),
            MB_OK | MB_ICONERROR);
    }
    updateStatusSummary();
}

// ── Window procedure ─────────────────────────────────────────────────────

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (uMsg == WM_CREATE) {
        auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->createControls(hwnd);
    } else {
        pThis = reinterpret_cast<MainWindow*>(
                    GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {

        case WM_GETMINMAXINFO: {
            auto mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = 480;
            mmi->ptMinTrackSize.y = 360;
            return 0;
        }

        case WM_SIZE:
            pThis->onSize(hwnd, (UINT)wParam,
                          LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_COMMAND:
            pThis->onCommand(hwnd, LOWORD(wParam),
                             (HWND)lParam, HIWORD(wParam));
            return 0;

        case WM_CONTEXTMENU:
            if ((HWND)wParam == pThis->m_hwndList)
                pThis->onContextMenu(hwnd,
                    GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_NOTIFY:
            pThis->onNotify(hwnd, (LPNMHDR)lParam);
            return 0;

        case WM_FIREWALL_COMPLETE:
            pThis->onFirewallComplete(hwnd, wParam, lParam);
            return 0;

        case WM_UNBLOCK_ALL_COMPLETE:
            pThis->onUnblockAllComplete(hwnd, wParam, lParam);
            return 0;

        case WM_CLEANUP_ORPHANS_COMPLETE:
            pThis->onCleanupOrphansComplete(hwnd, wParam, lParam);
            return 0;

        case WM_DESTROY:
            pThis->saveColumnState();
            if (pThis->m_hFont)     DeleteObject(pThis->m_hFont);
            if (pThis->m_hBoldFont) DeleteObject(pThis->m_hBoldFont);
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
