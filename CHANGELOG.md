# Changelog

All notable changes to this project will be documented in this file.

## [2.1.1] - 2026-07-29

### 🚀 New Features

#### ↔️ Drag Columns to Reorder
- Drag column headers to reorder them, just like Windows File Explorer
- `LVS_EX_HEADERDRAGDROP` extended style enables native drag-drop
- `HDN_ENDDRAG` notification disables auto-resize to preserve custom layout

#### 💾 Column State Persistence
- Column widths, order, sort column, and sort direction saved to Registry
- Stored under `HKCU\Software\APP-NETWORK_MANAGER` on `WM_DESTROY`
- Restored on next launch before populating the app list
- "Reset Columns Width" clears saved state for a fresh start

#### 💬 Balloon Tooltips on Action Buttons
- Hover over any action button (Block, Allow, Unblock All, Refresh, Exit)
- Balloon-style tooltips with translated button text
- Dynamic text via `LPSTR_TEXTCALLBACKW` + `TTN_GETDISPINFO` handler
- Tooltips update automatically when language is changed

#### 📊 Visual Sort Indicator in Status Bar
- Status bar shows current sort order:
  - `"Nombre ▲  |  3 of 45 blocked"`  (sorted by name, ascending)
  - `"Status ▼  |  3 of 45 blocked"` (sorted by status, descending)
- Column names translated according to selected language
- Updates automatically on column header click

#### 🔒 Minimum Window Size
- Window minimum size enforced: 480×360 pixels
- Prevents UI elements from overlapping when window is too small
- Handled via `WM_GETMINMAXINFO` message

### 🐛 Bug Fixes

#### Status Column Never Updated (Critical)
- **Root cause**: `getBlockedAppNames()` parsed `netsh` output searching for
  English strings `"Rule Name:"` and `"Action:"`. On non-English Windows
  (Spanish, French, Chinese, etc.), `netsh` outputs localized strings like
  `"Nombre de regla:"`, causing the parser to return **zero blocked apps**.
- **Fix**: Rewrote parser to be **locale-independent** — searches for `"Block_"`
  prefix (the rule name prefix we always use) in any line of netsh output.
- **Bonus**: `onFirewallComplete()` now modifies `m_blockedApps` **directly**
  (`insert` for block, `erase` for allow), bypassing netsh entirely for
  instantaneous status updates after block/allow operations.

#### Compilation Errors with Emoji in MinGW
- `\uD83D` / `\uDD04` / `\uDEAB` surrogate pairs not valid universal characters
  in MinGW-GCC — caused 39+ compilation errors across all language tables
- Fixed: replaced all `\uD83D\uDD04` (↻) with `L"↻"` and `\uD83D\uDEAB` (🚫)
  with `L"🚫"` as direct wide characters

#### CJK/Turkish Translation Warnings
- `\/` escape sequence in Chinese, Japanese, Korean translations (e.g., `{1}\/{2}`)
  generated 8 "unknown escape sequence" warnings — fixed by using `/` directly
  (no escape needed in wide string literals)

#### Missing `MainWindow::show()` Implementation
- Declaration existed in header but definition was missing — caused linker error
  `"undefined reference to MainWindow::show(int)"`
- Added implementation that calls `ShowWindow()` and `UpdateWindow()`

#### Language Combo Box Dropdown
- Combo box only showed 1 item at a time; user couldn't see other languages
- Fixed by setting `CB_SETMINVISIBLE` to 15 (total languages) via `SendMessage`

#### Duplicate Icon on Refresh Button
- Refresh button showed "🔒🔒" (two lock icons) instead of "🔄"
- Root cause: emoji surrogate pairs corrupted in MinGW string tables
- Fixed alongside the emoji replacement fix above

#### Font Size Too Small
- Application list and status column text was too small (font size 14)
- Increased header font from 14 to 16 and list font from 14 to 16

### ♻️ Architecture

#### Locale-Independent Firewall Detection
- `ApplicationManager::getBlockedAppNames()` completely rewritten
- Before: parsed English keywords ("Rule Name:", "Action:", "Block")
- After: searches for `"Block_"` prefix directly in any line of netsh output
- Works correctly on **any Windows language** — English, Spanish, Chinese, Arabic, etc.

#### Direct Status Updates (No netsh Query)
- `onFirewallComplete()` bypasses `populateBlockedStatus()` for single-app ops
- Block → `m_blockedApps.insert(safeName)`
- Allow → `m_blockedApps.erase(safeName)`
- Instant, reliable, and independent of netsh localization
- `populateBlockedStatus()` still used on Refresh for full resync

### 🛠️ Build Improvements

#### Step-by-Step Progress in build.bat
- Before: no output during compilation (user saw a frozen terminal for 30+ seconds)
- After: clear `[1/6]` through `[6/6]` progress for each step:
  ```
  [1/6] Compiling resources...
  [2/6] Compiling LanguageManager.cpp...
  [3/6] Compiling ApplicationManager.cpp...
  [4/6] Compiling MainWindow.cpp...
  [5/6] Compiling app_ui.cpp...
  [6/6] Linking (static build - this can take 30-60 seconds)...
  ```

### 📁 Files Changed

#### Modified Files
- `UI/MainWindow.h` — m_hwndTooltip, m_tooltipBuffer, m_lastFirewallKey,
  m_minWindowSize, saveColumnState(), loadColumnState()
- `UI/MainWindow.cpp` — Tooltip creation, TTN_GETDISPINFO, Registry save/load,
  column drag (HDN_ENDDRAG), sort indicator in status bar, direct m_blockedApps
  manipulation, locale-independent status bar sort text, WM_GETMINMAXINFO,
  applySort() after firewall completion, InvalidateRect after sort
- `UI/ApplicationManager.cpp` — Rewrote getBlockedAppNames() to locale-independent
  parser (search for "Block_" prefix instead of English keywords)
- `UI/LanguageManager.cpp` — Fixed `\/` escape sequences in CJK/Turkish strings,
  replaced surrogate pairs with direct emoji characters
- `UI/build.bat` — Step-by-step progress [1/6]-[6/6]
- `CHANGELOG.md` — This update
- `README.md` — Updated

## [2.1.0] - 2026-07-29

### 🚀 New Features

#### 🔓 Unblock All Button
- New "Unblock All" button alongside Block/Allow/Exit
- Iterates all blocked apps and removes their firewall rules in a background thread
- Shows success/failure count on completion
- Also available from the right-click context menu

#### 🧹 Orphaned Rules Detection & Cleanup
- Detects firewall rules for apps that are no longer installed
- Computed by comparing `m_blockedApps` against `m_apps` (sanitized names)
- Shows orphan count in status bar: `"3 of 45 blocked  |  2 orphaned"`
- "Cleanup N Orphaned Rule(s)" in context menu (only visible when orphans exist)
- Removes stale rules via background thread with atomic guard

#### 📊 Column Sorting
- Click "Application Name" header → sort alphabetically (A-Z / Z-A)
- Click "Status" header → sort by blocked status (blocked first)
- Sort arrows (▲▼) on the header column via `HDF_SORTUP` / `HDF_SORTDOWN`
- Case-insensitive comparison with `_stricmp`

#### ↔️ Customizable Column Widths
- Drag column dividers in the header to resize columns
- Widths persist across window resizes (no longer overwritten by `onSize`)
- `HDN_ENDTRACK` notification sets `m_columnsUserSet = true`
- "Reset Columns Width" option in right-click context menu restores defaults

#### 💬 Tooltips on Blocked Apps
- Hover over a blocked app to see its firewall rule name
- Tooltip format: `"Firewall rule: Block_<AppName>"`
- Handled via `LVN_GETINFOTIP` notification (LVS_EX_INFOTIP already enabled)
- Only shown for blocked apps — allowed apps get no custom tooltip

#### 🔍 Empty Filter Indicator
- When search filter produces 0 results, the header label changes to:
  `"No applications match your search"` (translated to all 15 languages)
- New `EmptyFilterText` string ID (33rd entry in all language tables)

### ⚡ Performance & Safety Improvements

#### WM_SETREDRAW Batching
- `refreshApplicationList()` and `applyFilter()` wrap bulk delete/insert in
  `SendMessage(WM_SETREDRAW, FALSE/TRUE)` + `InvalidateRect`
- Eliminates flickering during full list rebuilds (especially with 50+ apps)

#### Atomic Guard for Firewall Operations
- `std::atomic<bool> m_firewallRunning` prevents concurrent firewall commands
- `compare_exchange_strong` on entry, `store(false)` on ALL exit paths
- Shows "operation in progress" message box if a second operation is attempted
- Critical fix: flag is cleared on early-return paths (no selection, empty path, no executables)

#### Unicode & Timeout Improvements
- `FirewallHelper.h`: `CreateProcessA` → `CreateProcessW` with `MultiByteToWideChar`
- Added 30-second `WaitForSingleObject` timeout + `TerminateProcess` on timeout
- `noexcept` specifiers on all firewall helper functions

#### Memory Pre-Allocation
- `ApplicationManager.cpp`: `apps.reserve(256)` for registry scan results
- `RunCommandAndCaptureOutput`: `result.reserve(16384)` for netsh output buffer
- Pipe read buffer increased from 4K to 8K

### ♻️ Code Quality

#### StringHelper.h — Shared Sanitization
- Duplicated `SanitizeForCmd()` removed from `app.cpp`, `app_en.cpp`, `ApplicationManager.cpp`
- New `StringHelper.h` (header-only, inline, noexcept) at project root
- ~50 lines of duplicate code eliminated
- All 9 call sites migrated to the shared function

#### Console App Safety
- `system("pause")` replaced with `cin.get()` / `cin.ignore(numeric_limits...)` + `cin.get()`
- Added `#include <limits>` for `numeric_limits<streamsize>`
- No more spawning `cmd.exe` just to pause

#### Certificates & SmartScreen
- `sign.bat` — Script to sign the executable with a self-signed certificate
- `INSTALL.md` — Step-by-step guide to bypass SmartScreen

### 🐛 Bug Fixes

- **Atomic guard permanent lock**: `m_firewallRunning` stayed `true` after early-return
  paths (no selection, empty path, no executables), blocking all future operations
  until app restart. Fixed with `store(false)` on all 3 early return paths.

### 📁 Files Changed

#### New Files
- `StringHelper.h` — Shared SanitizeForCmd (header-only)
- `sign.bat` — Certificate signing script
- `INSTALL.md` — Signing & installation guide

#### Modified Files
- `UI/MainWindow.h` — m_columnsUserSet, m_orphanedApps, IDs 108-110, WM_APP+2/+3
- `UI/MainWindow.cpp` — Context menu, sort, orphan detection, tooltips, WM_SETREDRAW,
  atomic guard, column resize, empty filter, reset columns
- `UI/LanguageManager.h` — BtnUnblockAll, EmptyFilterText enums (now StringId::Count = 33)
- `UI/LanguageManager.cpp` — +2 translation entries (now 33 strings × 15 langs = 495 total)
- `UI/ApplicationManager.cpp` — reserve(), CreateProcessW, noexcept
- `app.cpp` — system(pause) removed, cin.ignore fix, StringHelper.h include
- `app_en.cpp` — Same fixes as app.cpp
- `FirewallHelper.h` — Unicode, timeout, noexcept
- `CHANGELOG.md` — This update
- `README.md` — Updated

## [2.0.0] - 2026-07-29

### 🏗️ Major New Features

#### 🌐 Multi-language Support (15 languages)
- Full internationalization via new `LanguageManager` class
- Languages: English, Spanish, French, German, Italian, Portuguese, Dutch,
  Russian, Chinese (Simplified), Japanese, Korean, Arabic, Turkish, Polish, Swedish
- Auto-detects system language on startup via `GetUserDefaultUILanguage()`
- In-app language switcher (combo box) for instant language change
- All UI strings (31 per language) translated — buttons, columns, messages, status
- No external files — all translations embedded in the executable

#### 🔍 Application Search & Filter
- Real-time search box filters the application list as you type
- Case-insensitive matching against application names

#### 🔴 Blocked Status Column
- New "Status" column in the ListView shows 🔴 Blocked / 🟢 Allowed
- Queries Windows Firewall via `netsh advfirewall firewall show rule name=all dir=out`
- Pipe-based stdout capture and output parsing
- Automatically refreshes after Block/Allow operations

#### 🔄 Registry Refresh Button
- "↻ Refresh" button re-scans the Windows Registry for newly installed apps
- Re-queries firewall rules to update blocked status
- Clears search filter on refresh

#### ⚡ Async Firewall Operations
- Firewall commands run in background threads via `std::thread`
- UI stays fully responsive during blocking/allowing operations
- Buttons disabled during operation to prevent double-clicks
- Working indicator "⏳ Working..." shown during execution

#### 🖱️ Double-Click to Block
- Double-clicking any application in the list triggers Internet block
- Faster workflow for common action

#### 📊 Status Bar
- Professional status bar at the bottom of the window
- Contextual messages: scanning, working, success/failure with ✓/✗ indicators

### 🔒 Security Fixes

#### Command Injection Prevention (all versions)
- Added `sanitizeForFirewall()` / `SanitizeForCmd()` to strip double quotes from
  application names and executable paths before building netsh commands

#### system() → CreateProcess (all versions)
- **GUI**: Firewall commands execute via `CreateProcess` with `CREATE_NO_WINDOW`
  in a background thread — no console flash, no UI freeze
- **Console versions**: Same `CreateProcess` approach, with proper exit code
  checking and success/failure feedback (was previously silently ignored)
- Extracted into shared header `FirewallHelper.h` to eliminate code duplication

### 🐛 Bug Fixes

#### ApplicationManager: DisplayIcon Fallback
- Registry enumeration now falls back to `DisplayIcon` value when `InstallLocation`
  is missing, matching the behavior of the console versions
- Added `ExtractFolderFromPath()` implementation (was declared but missing)

#### GDI Font Handle Leak (MainWindow)
- `CreateFont()` was called but never paired with `DeleteObject()`
- Fixed: HFONT stored as member, properly freed in `WM_DESTROY` handler
- Also added `m_hBoldFont` for header label, properly freed

### ♻️ Code Quality & Architecture

#### Dead Code Removal
- `app_ui.cpp`: Removed ~150 lines of legacy code — old `WndProc`, `gApps` global,
  duplicate `Application` struct, duplicate registry functions, unused control
  ID macros, and unused `GetExecutableFromFolder()` function
- `MainWindow.cpp`: Removed unused `updateLayout()` method

#### Shared FirewallHelper.h
- Created header-only `FirewallHelper.h` with `inline ExecuteFirewallCommand()`
- Eliminates duplicate CreateProcess logic across `app.cpp`, `app_en.cpp`, and
  `MainWindow.cpp` (3 copies → 1 shared)
- Header-only design — no additional compilation unit needed

#### Separation of Concerns
- `populateBlockedStatus()` vs `updateStatusColumn()` split to avoid re-querying
  the firewall on every search keystroke (performance optimization)
- `startFirewallOperation()` extracted from `onCommand()` for cleaner code

### 🎨 UI/UX Improvements

- Larger default window size (580×480)
- Centered action buttons with improved spacing
- Responsive column widths that auto-resize with window
- Clearer success/error messages with Administrator guidance
- Professional sunken status bar with ✓/✗ feedback
- Emoji-enhanced button labels (🛡️ Block Internet, ✅ Allow Internet)
- ListView with `LVS_EX_FULLROWSELECT`, `LVS_EX_DOUBLEBUFFER`, `LVS_EX_INFOTIP`
- Improved error messages with actionable instructions
- Refresh button disabled during firewall operations
- Language combo box disabled during firewall operations

### 📁 Files Changed

#### New Files
- `UI/LanguageManager.h` — Language enum, StringId enum, singleton class
- `UI/LanguageManager.cpp` — Translation tables (15 languages × 31 strings)
- `FirewallHelper.h` — Shared CreateProcess helper (header-only)

#### Modified Files
- `UI/ApplicationManager.h` — Added `getBlockedAppNames()`, `RunCommandAndCaptureOutput()`
- `UI/ApplicationManager.cpp` — DisplayIcon fallback, firewall query implementation
- `UI/MainWindow.h` — New controls (search, refresh, language combo, status bar, working label)
- `UI/MainWindow.cpp` — Complete rewrite: i18n, search, async ops, status column, layout
- `UI/app_ui.cpp` — Dead code removed (legacy WndProc, gApps, duplicates)
- `UI/build.bat` — Added LanguageManager compilation
- `app.cpp` — Sanitization, CreateProcess, shared helper
- `app_en.cpp` — Same fixes as app.cpp

## [1.0.1] - 2025-03-17

### Fixed
- Fixed application icon not displaying in window title bar
- Improved static linking for better portability
- Resolved dependencies issues for standalone executable
- Enhanced icon resource integration

## [1.0.0] - 2025-03-17

### Added
- Initial release
- Console application version (app.cpp)
  - Spanish language support
  - English language support
  - Registry scanning for installed applications
  - Automatic executable detection
  - Firewall rule management
- GUI Version (UI folder)
  - Modern interface with Windows Common Controls
  - Responsive layout
  - Improved user experience
  - Application icon support
  - Better error handling and feedback
  - Simplified application selection
  - Automatic executable detection

### Technical Updates
- Implementation of ApplicationManager class
- Separation of concerns in GUI version
- Modern C++20 features usage
- Windows Registry integration
- Proper Unicode support
- Administrative privileges handling

### Fixed
- Registry path handling for both 32-bit and 64-bit applications
- Memory management improvements
- Window scaling issues
- Unicode text display problems
