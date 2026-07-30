#pragma once
#include <windows.h>
#include <sddl.h>
#include <shellapi.h>

// Check whether the current process is running with Administrator privileges.
// Uses TokenMembership check against the built-in Administrators group.
inline bool IsRunAsAdmin() noexcept {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return (isAdmin != FALSE);
}
