#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <set>
#include <atomic>
#include "ApplicationManager.h"
#include "LanguageManager.h"

class MainWindow {
public:
    static MainWindow& getInstance();
    bool create(HINSTANCE hInstance);
    void show(int nCmdShow);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    MainWindow() = default;
    void createControls(HWND hwnd);
    void onSize(HWND hwnd, UINT flag, int width, int height);
    void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void onNotify(HWND hwnd, LPNMHDR nmhdr);
    void onFirewallComplete(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void refreshApplicationList();
    void applyFilter();
    void setStatusText(const std::wstring& text);
    void updateHeaderLabel();
    int getSelectedAppIndex();
    void startFirewallOperation(HWND hwnd, int id);
    void populateBlockedStatus();
    void updateStatusColumn();
    void refreshUIStrings();
    void onLanguageChanged();
    void onContextMenu(HWND hwnd, int screenX, int screenY);
    void onColumnClick(int column);
    void applySort();
    void updateStatusSummary();
    void startUnblockAllOperation(HWND hwnd);
    void onUnblockAllComplete(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void computeOrphanedApps();
    void startCleanupOrphans(HWND hwnd);
    void onCleanupOrphansComplete(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void saveColumnState();
    void loadColumnState();

    HWND m_hwnd = nullptr;
    HWND m_hwndList = nullptr;
    HWND m_hwndBlockBtn = nullptr;
    HWND m_hwndAllowBtn = nullptr;
    HWND m_hwndExitBtn = nullptr;
    HWND m_hwndUnblockAllBtn = nullptr;
    HWND m_hwndSearchEdit = nullptr;
    HWND m_hwndRefreshBtn = nullptr;
    HWND m_hwndLangCombo = nullptr;
    HWND m_hwndHeaderLabel = nullptr;
    HWND m_hwndWorkingLabel = nullptr;
    HWND m_hwndStatusBar = nullptr;
    HWND m_hwndTooltip = nullptr;

    std::vector<ApplicationManager::Application> m_apps;
    std::set<std::string> m_blockedApps;
    std::set<std::string> m_orphanedApps;
    HFONT m_hFont = nullptr;
    HFONT m_hBoldFont = nullptr;

    int  m_sortColumn   = 0;   // Default: sort by Application Name ascending
    bool m_sortAscending = true;

    bool m_columnsUserSet = false;
    std::atomic<bool> m_firewallRunning{false};
    std::string m_lastFirewallKey;  // Sanitized app name of last block/allow operation
    wchar_t m_tooltipBuffer[256];

    // Control IDs
    static constexpr int ID_LISTVIEW     = 101;
    static constexpr int ID_BLOCK        = 102;
    static constexpr int ID_ALLOW        = 103;
    static constexpr int ID_EXIT         = 104;
    static constexpr int ID_SEARCH_EDIT  = 105;
    static constexpr int ID_REFRESH      = 106;
    static constexpr int ID_LANG_COMBO   = 107;
    static constexpr int ID_UNBLOCK_ALL   = 108;
    static constexpr int ID_CLEANUP_ORPHANS = 109;
    static constexpr int ID_RESET_COLUMNS   = 110;
    // Custom messages for async firewall command completion
    static constexpr UINT WM_FIREWALL_COMPLETE       = WM_APP + 1;
    static constexpr UINT WM_UNBLOCK_ALL_COMPLETE    = WM_APP + 2;
    static constexpr UINT WM_CLEANUP_ORPHANS_COMPLETE = WM_APP + 3;
};
