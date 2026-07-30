# NETCONTROL-APP

A Windows application to manage Internet access for installed applications through Windows Firewall rules.

![Languages](https://img.shields.io/badge/languages-15-brightgreen)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

- **List all installed applications** — Scans Windows Registry (HKLM, WOW6432Node, HKCU)
- **Block/Allow Internet access** — Creates/removes Windows Firewall rules via `netsh`
- **🔴 Blocked status detection** — Locale-independent: works on any Windows language
- **🔍 Real-time search** — Filter applications as you type, with empty-state indicator
- **🔄 Refresh button** — Re-scan registry for newly installed apps without restarting
- **↕️ Column sorting** — Click column headers to sort by name or blocked status
- **📊 Sort indicator in status bar** — Shows current sort order ("Name ▲ | 3 of 45 blocked")
- **🖱️ Context menu** — Right-click for Block / Allow / Unblock All / Cleanup Orphans / Reset Columns
- **🔓 Unblock All** — Remove all `Block_` firewall rules at once
- **🧹 Orphaned rules detection** — Finds and cleans stale firewall rules for uninstalled apps
- **↔️ Drag columns to reorder** — Like Windows File Explorer, drag headers to rearrange
- **↔️ Customizable column widths** — Drag dividers, persists across window resizes
- **💾 Column state persistence** — Widths, order, and sort saved to Registry on close
- **💬 Tooltips on action buttons** — Balloon tooltips on Block, Allow, Refresh, Exit
- **💬 Tooltips on blocked apps** — Hover to see the firewall rule name
- **🖱️ Double-click to block** — Quick action on any application
- **🌐 15 languages** — With auto-detect and instant in-app switching
- **Available in Console and GUI versions**
- **Administrative privileges handling** — Automatic UAC elevation
- **Async operations** — Firewall commands run in background thread with atomic guard
- **Clean and modern interface** — Professional status bar, responsive layout, flicker-free

## 🌐 Languages

| Language | Native name | Auto-detected |
|----------|-------------|:---:|
| English | English | Default |
| Spanish | Español | ✅ |
| French | Français | ✅ |
| German | Deutsch | ✅ |
| Italian | Italiano | ✅ |
| Portuguese | Português | ✅ |
| Dutch | Nederlands | ✅ |
| Russian | Русский | ✅ |
| Chinese (Simplified) | 中文 | ✅ |
| Japanese | 日本語 | ✅ |
| Korean | 한국어 | ✅ |
| Arabic | العربية | ✅ |
| Turkish | Türkçe | ✅ |
| Polish | Polski | ✅ |
| Swedish | Svenska | ✅ |

The app detects your Windows UI language on startup. You can switch languages at any time via the 🌐 dropdown in the toolbar.

## 📸 Interface

```
┌──────────────────────────────────────────────────────────────────┐
│ [🔍 Filter applications...      ] [↻ Refresh] [🌐 English ▼]    │
│ Installed Applications (12 of 45)          ⏳ Working...         │
│ ┌───────────────────────────────────────────┬──────────────┐     │
│ │ Application Name                    ▲    │ Status   ▼    │     │
│ ├───────────────────────────────────────────┼──────────────┤     │
│ │ Discord                                  │ 🔴 Blocked   │     │
│ │ Firefox                                  │ 🔴 Blocked   │     │
│ │ Google Chrome                            │ 🟢 Allowed   │     │
│ │ Steam                                    │ 🟢 Allowed   │     │
│ │ ...                                      │ ...          │     │
│ └───────────────────────────────────────────┴──────────────┘     │
│ [🚫 Block] [✅ Allow] [🔓 Unblock All] [↻ Refresh] [❌ Exit]   │
├──────────────────────────────────────────────────────────────────┤
│ Name ▲  |  3 of 45 applications currently blocked  |  2 orphaned│
└──────────────────────────────────────────────────────────────────┘

> 💬 *Hover a blocked app → tooltip shows "Firewall rule: Block_<name>"*
> 💬 *Hover any action button → balloon tooltip with translated text*
> 🖱️ *Right-click any app → context menu with all actions*
> ↔️ *Drag column headers to reorder columns like File Explorer*
> ↔️ *Drag header dividers → custom column widths*
> 🔍 *Type to filter → "No applications match your search" when empty*
> 📊 *Status bar shows current sort order: Name ▲ / Name ▼ / Status ▲ / Status ▼*

## 🖥️ Versions

| Version | Interface | Files | Languages |
|---------|-----------|-------|-----------|
| **GUI** | Modern Win32 GUI | `UI/` folder | 15 (auto-detect + combo box) |
| **Console (English)** | Command-line | `app_en.cpp` | English |
| **Console (Spanish)** | Command-line | `app.cpp` | Spanish |

> **New in v2.1.1:** Drag columns to reorder, column state persistence via Registry,
> balloon tooltips on all action buttons, visual sort indicator in status bar,
> locale-independent firewall detection (works in any language),
> instant status updates (no netsh query on block/allow),
> fixed emoji compilation errors, fixed CJK escape sequences, improved build output

## 📁 Project Structure

```
APP-NETWORK_MANAGER/
├── FirewallHelper.h          ← Shared firewall command helper (CreateProcessW, Unicode)
├── StringHelper.h            ← Shared sanitization helper (SanitizeForCmd)
├── sign.bat                  ← Certificate signing script (anti-SmartScreen)
├── INSTALL.md                ← Signing & installation guide
├── app.cpp                   ← Console version (Spanish)
├── app_en.cpp                ← Console version (English)
├── UI/
│   ├── ApplicationManager.h  ← Business logic: registry scanning, firewall queries
│   ├── ApplicationManager.cpp
│   ├── MainWindow.h          ← GUI main window (atomic guard, sort, orphans, columns)
│   ├── MainWindow.cpp        ← UI layout, controls, event handlers, async ops
│   ├── LanguageManager.h     ← Multilingual support (15 languages, 33 strings)
│   ├── LanguageManager.cpp   ← Translation tables (495 entries)
│   ├── app_ui.cpp            ← GUI entry point (WinMain + admin check)
│   ├── build.bat             ← Build script (MinGW-w64)
│   └── resources.rc          ← Application icon, version info
├── CHANGELOG.md
├── LICENSE                   ← MIT License
└── README.md
```

### Helpers compartidos

| Helper | Location | Purpose |
|--------|----------|---------|
| `FirewallHelper.h` | Project root | Execute firewall commands (Unicode, timeout 30s, noexcept) |
| `StringHelper.h` | Project root | Sanitize strings against command injection |

### Flujo de datos

```
Registry ──→ ApplicationManager ──→ MainWindow (m_apps)
                ↓                        ↓
           netsh query (Refresh)   populateBlockedStatus()
                ↓                        ↓
           m_blockedApps ────────→ computeOrphanedApps()
                ↓                        ↓
      ┌─ Block/Allow (direct) ────→ insert/erase m_blockedApps
      │                              (no netsh needed!)
      ↓
           m_orphanedApps ─────── → updateStatusSummary()
                                      "Name ▲ | 3 of 45 blocked  |  2 orphaned"
```

> **Note:** Block/Allow operations update `m_blockedApps` **directly** without
> querying netsh. Full netsh resync only happens on Refresh or app startup.

## 📋 Requirements

- **Windows 7 / 8 / 10 / 11** (x86 or x64)
- **Administrative privileges** (required for firewall rule changes)
- **MinGW-w64** (for compilation from source)
- **C++20 compatible compiler**

## 🔧 Building

### GUI Version

Simply run the build script from the `UI/` folder:

```batch
cd UI
build.bat
```

Or manually:

```bash
cd UI
windres resources.rc -O coff -o resources.res
g++ -std=c++20 -c LanguageManager.cpp -o LanguageManager.o -static
g++ -std=c++20 -c ApplicationManager.cpp -o ApplicationManager.o -static
g++ -std=c++20 -c MainWindow.cpp -o MainWindow.o -static
g++ -std=c++20 -c app_ui.cpp -o app_ui.o -static
g++ LanguageManager.o ApplicationManager.o MainWindow.o app_ui.o resources.res \
    -o APP-NETWORK_MANAGER.exe \
    -mwindows -static-libgcc -static-libstdc++ -static \
    -lstdc++fs -lpthread -lcomctl32 -lole32 -luuid -loleaut32
```

### Console Version

```bash
g++ -std=c++20 app.cpp -o NETCONTROL.exe -lstdc++fs -lpthread
g++ -std=c++20 app_en.cpp -o NETCONTROL_EN.exe -lstdc++fs -lpthread
```

> **Note:** The console versions are standalone single-file builds. The GUI version
> uses multiple files and requires the full `UI/` directory.

## 🚀 Usage

1. **Run as Administrator** — The app will prompt for elevation if needed
2. **Browse applications** — Use the search box to filter, or scroll the list
3. **Check status** — The "Status" column shows 🔴 Blocked or 🟢 Allowed
4. **Block/Allow** — Select an app and click the button, or double-click to block
5. **Refresh** — Click ↻ Refresh after installing new apps
6. **Language** — Use the 🌐 dropdown to switch languages at any time

## 🔒 Architecture

### Security

- All netsh commands execute via `CreateProcess` with `CREATE_NO_WINDOW` (no `system()`)
- Application names and paths are sanitized (double quotes stripped) before inclusion
- Asynchronous execution prevents UI hangs

### Firewall Rule Detection

The app queries Windows Firewall via:
```bash
netsh advfirewall firewall show rule name=all dir=out
```
Output is captured through anonymous pipes, parsed to extract rules with names
starting with `Block_`. The parser is **locale-independent** — it searches for
`"Block_"` prefix directly instead of English keywords like `"Rule Name:"` or
`"Action:"`. This means it works correctly on **any Windows language**.

On block/allow operations, the status is updated **directly** in memory without
querying netsh, making it instant and reliable regardless of system language.

### Column Sorting

Click any column header to sort:
- **Application Name** — Alphabetical (case-insensitive, `_stricmp`)
- **Status** — Blocked apps first, then alphabetical by name
- Click again to toggle ascending/descending with ▲▼ arrow indicators
- **Status bar** shows current sort order (e.g., `"Name ▲"` or `"Status ▼"`)

### Column Reordering

Drag any column header to reorder columns, just like Windows File Explorer.
Column order is saved to the Registry and restored on next launch.

### Button Tooltips

Hover over any action button to see a balloon-style tooltip with the translated
button text. Tooltips update automatically when the language is changed.

### Column State Persistence

Column widths, order, sort column, and sort direction are saved to
`HKCU\Software\APP-NETWORK_MANAGER` when the app closes and restored on next launch.
Use "Reset Columns Width" in the right-click context menu to start fresh.

### Multilingual Architecture

All 33 UI strings are stored in a static 2D array (`Language × StringId`). Format strings
support `{1}` and `{2}` placeholders for dynamic content (e.g., "Blocking {1}..." →
"Blocking Firefox..."). Language is detected via `GetUserDefaultUILanguage()`.

## 🤝 Contributing

Feel free to submit issues and pull requests. To add a new language:
1. Add a value to the `Language` enum in `LanguageManager.h`
2. Add a translation row in `LanguageManager.cpp` (copy the English row and translate)
3. Add the language code to `detectSystemLanguage()` in `LanguageManager.cpp`

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
