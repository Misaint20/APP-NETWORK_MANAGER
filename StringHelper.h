#pragma once
#include <string>

// ─────────────────────────────────────────────────────────────────────────
//  SanitizeForCmd — Removes double-quote characters to prevent command
//  injection when building netsh command lines from user-visible app names.
//
//  Usage:
//     std::string safe = SanitizeForCmd(rawAppName);
//     // → "Google Chrome" stays "Google Chrome"
//     // → `Fire"fox` becomes `Firefox`
// ─────────────────────────────────────────────────────────────────────────
inline std::string SanitizeForCmd(const std::string& input) noexcept {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        if (c != '"')
            result += c;
    }
    return result;
}
