#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────
//  ExecuteFirewallCommand — Runs a netsh (or any) command via CreateProcessW
//  with CREATE_NO_WINDOW so no console window flashes.
//
//  Returns:  exit code (0 = success)
//           -1 if the process could not be created.
//
//  Usage:
//     int rc = ExecuteFirewallCommand("netsh advfirewall ...");
//     if (rc == 0) { /* success */ }
// ─────────────────────────────────────────────────────────────────────────
inline int ExecuteFirewallCommand(const std::string& command) noexcept {
    // Convert UTF-8 command line to UTF-16 (required by CreateProcessW)
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), nullptr, 0);
    if (wideLen <= 0) return -1;

    std::wstring wideCmd(static_cast<size_t>(wideLen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.data(), static_cast<int>(command.size()), &wideCmd[0], wideLen);

    // CreateProcessW may modify the buffer → mutable copy
    std::vector<wchar_t> cmdLine(wideCmd.begin(), wideCmd.end());
    cmdLine.push_back(L'\0');

    STARTUPINFOW si        = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    BOOL created = CreateProcessW(
        nullptr,            // use command line (no app name)
        cmdLine.data(),     // mutable command line
        nullptr,            // process handle not inheritable
        nullptr,            // thread handle not inheritable
        FALSE,              // no handle inheritance
        CREATE_NO_WINDOW,   // hide the console window
        nullptr,            // use parent's environment
        nullptr,            // use parent's current directory
        &si, &pi);

    if (!created)
        return -1;

    // Wait with a timeout safeguard (30 seconds)
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000);

    DWORD exitCode = 1;
    if (waitResult == WAIT_OBJECT_0)
        GetExitCodeProcess(pi.hProcess, &exitCode);
    else
        TerminateProcess(pi.hProcess, 1);  // timeout -> kill

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exitCode);
}
