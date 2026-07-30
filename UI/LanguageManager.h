#pragma once
#include <windows.h>
#include <string>
#include <vector>

// ── Supported languages ──────────────────────────────────────────────────
enum class Language {
    English,       // 0 – default
    Spanish,       // 1
    French,        // 2
    German,        // 3
    Italian,       // 4
    Portuguese,    // 5
    Dutch,         // 6
    Russian,       // 7
    Chinese,       // 8 – Simplified
    Japanese,      // 9
    Korean,        // 10
    Arabic,        // 11
    Turkish,       // 12
    Polish,        // 13
    Swedish,       // 14
    Count
};

// ── String IDs (all visible UI strings) ──────────────────────────────────
enum class StringId {
    WindowTitle,
    Refresh,
    HeaderLabel,        // {1}=shown  {2}=total
    Working,
    ColumnAppName,
    ColumnStatus,
    StatusBlocked,
    StatusAllowed,
    BtnBlock,
    BtnAllow,
    BtnExit,
    Scanning,
    ScanningFirewall,
    Ready,              // {1}=count
    BlockingAccess,     // {1}=app name
    AllowingAccess,     // {1}=app name
    AccessBlocked,
    AccessAllowed,
    AccessBlockFailed,
    AccessAllowFailed,
    SelectAppFirst,
    InstallPathNotFound,
    NoExecutablesFound,
    SuccessBlockMsg,
    SuccessAllowMsg,
    FailBlockMsg,
    FailAllowMsg,
    InfoTitle,
    ErrorTitle,
    SuccessTitle,
    BlockedSummary, // {1}=blocked count  {2}=total
    BtnUnblockAll,
    EmptyFilterText, // shown when search filter has no matches
    Count
};

// ── LanguageManager singleton ────────────────────────────────────────────
class LanguageManager {
public:
    static LanguageManager& getInstance();

    Language getLanguage() const               { return m_lang; }
    void     setLanguage(Language lang)        { m_lang = lang;  }

    // Human-readable language name for the combo box (e.g. "English", "Español")
    static const wchar_t* getLanguageName(Language lang);

    // Translate a simple string ID
    const std::wstring& getString(StringId id) const;

    // Translate with one replacement (e.g. "Blocking {1}..." → "Blocking Firefox...")
    std::wstring getStringF(StringId id, const std::wstring& arg1) const;

    // Translate with two replacements (e.g. "{1} of {2}" → "12 of 45")
    std::wstring getStringF(StringId id, const std::wstring& arg1, const std::wstring& arg2) const;

    // Auto-detect from system UI language (GetUserDefaultUILanguage)
    static Language detectSystemLanguage();

private:
    LanguageManager();
    Language m_lang = Language::English;

    using Table = std::wstring[static_cast<int>(StringId::Count)];
    static const Table s_translations[];
};
