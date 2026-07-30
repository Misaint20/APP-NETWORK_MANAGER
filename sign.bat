@echo off
title APP-NETWORK_MANAGER — Code Signing Tool
chcp 65001 >nul

:: ─────────────────────────────────────────────────────────────────────
::  sign.bat — Self-signed code signing for APP-NETWORK-MANAGER
:: ─────────────────────────────────────────────────────────────────────
::  This script generates a self-signed certificate and signs the .exe.
::  For users to trust it, they must install 'certificate-public.cer'
::  into their Trusted Root Certification Authorities store.
::
::  Prerequisites:
::    1. Windows SDK (for signtool.exe)
::    2. Run as Administrator (to create certificate)
::    3. Build the project first with build.bat
:: ─────────────────────────────────────────────────────────────────────

setlocal enabledelayedexpansion

:: Configuration — change these values as needed
set CERT_SUBJECT=CN=APP-NETWORK_MANAGER, O=NetControl, C=US
set CERT_PASSWORD=NetControl2026
set CERT_FILE=certificate.pfx
set CERT_PUBLIC_FILE=certificate-public.cer
set EXE_NAME=APP-NETWORK_MANAGER.exe
set EXE_DIR=UI

echo ╔══════════════════════════════════════════════════════════════╗
echo ║     APP-NETWORK-MANAGER — Code Signing Tool                ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

:: ── Check admin rights ────────────────────────────────────────────
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [⚠] This script must be run as Administrator.
    echo     Right-click ^> "Run as administrator"
    echo.
    pause
    exit /b 1
)

:: ── Check if signtool exists ──────────────────────────────────────
where signtool >nul 2>&1
if %errorlevel% neq 0 (
    echo [⚠] signtool.exe not found in PATH.
    echo     Install Windows SDK or Visual Studio Build Tools.
    echo     Typical paths:
    echo       C:\Program Files (x86)\Windows Kits\10\bin\10.0.xxxxx.0\x64\
    echo.
    echo     Or run from "Developer Command Prompt for VS 2022"
    echo.
    pause
    exit /b 1
)

:: ── Check if EXE exists ───────────────────────────────────────────
if not exist "%EXE_DIR%\%EXE_NAME%" (
    echo [⚠] %EXE_NAME% not found in %EXE_DIR%\.
    echo     Run build.bat first to compile the project.
    echo.
    pause
    exit /b 1
)

echo [1/5] Checking for existing certificate...
if exist "%CERT_FILE%" (
    echo   ✓  Certificate found: %CERT_FILE%
    set CERT_EXISTS=1
) else (
    echo   •  No existing certificate. Will create a new one.
    set CERT_EXISTS=0
)

:: ── Generate self-signed certificate ──────────────────────────────
if %CERT_EXISTS%==0 (
    echo.
    echo [2/5] Generating self-signed certificate...
    echo       Subject: %CERT_SUBJECT%
    
    powershell -Command ^
        "$cert = New-SelfSignedCertificate ^
            -Type CodeSigningCert ^
            -Subject '%CERT_SUBJECT%' ^
            -CertStoreLocation 'Cert:\CurrentUser\My' ^
            -HashAlgorithm SHA256 ^
            -KeyLength 2048 ^
            -NotAfter (Get-Date).AddYears(5); ^
         $pwd = ConvertTo-SecureString -String '%CERT_PASSWORD%' -Force -AsPlainText; ^
         Export-PfxCertificate -Cert $cert -FilePath '%CERT_FILE%' -Password $pwd; ^
         Export-Certificate -Cert $cert -FilePath '%CERT_PUBLIC_FILE%'" >nul 2>&1
    
    if !errorlevel! equ 0 (
        echo   ✓  Certificate generated successfully.
        echo   📄 Private key: %CERT_FILE%
        echo   📄 Public key:  %CERT_PUBLIC_FILE%
    ) else (
        echo   [✗] Failed to generate certificate.
        pause
        exit /b 1
    )
) else (
    echo.
    echo [2/5] Using existing certificate: %CERT_FILE%
    
    :: Re-export public key from existing PFX
    powershell -Command ^
        "$cert = [System.Security.Cryptography.X509Certificates.X509Certificate2]::new( ^
            '%CERT_FILE%', '%CERT_PASSWORD%'); ^
         Export-Certificate -Cert $cert -FilePath '%CERT_PUBLIC_FILE%'" >nul 2>&1
    
    if !errorlevel! equ 0 (
        echo   ✓  Public key re-exported.
    )
)

:: ── Sign the executable ───────────────────────────────────────────
echo.
echo [3/5] Signing %EXE_NAME%...

signtool sign ^
    /f "%CERT_FILE%" ^
    /p %CERT_PASSWORD% ^
    /fd SHA256 ^
    /tr http://timestamp.digicert.com ^
    /td SHA256 ^
    "%EXE_DIR%\%EXE_NAME%"

if %errorlevel% equ 0 (
    echo   ✓  Executable signed successfully.
) else (
    echo   [✗] Signing failed. Check certificate and try again.
    pause
    exit /b 1
)

:: ── Verify signature ──────────────────────────────────────────────
echo.
echo [4/5] Verifying signature...

signtool verify /v /pa "%EXE_DIR%\%EXE_NAME%" >nul 2>&1

if %errorlevel% equ 0 (
    echo   ✓  Signature verified.
) else (
    echo   ⚠  Verification warning (expected for self-signed certs).
    echo      The signature exists but Windows doesn't trust it yet.
)

:: ─── Final instructions ───────────────────────────────────────────
echo.
echo [5/5] Done!
echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║  ✅  %EXE_NAME% has been signed!                        ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.
echo 📄  Signed executable: %EXE_DIR%\%EXE_NAME%
echo 📄  Certificate (PFX):  %CERT_FILE%
echo 📄  Public key (CER):   %CERT_PUBLIC_FILE%
echo.
echo ═══ For end users ═══════════════════════════════════════════════
echo.
echo To avoid SmartScreen warnings, users must install the certificate:
echo.
echo   Option A — One command (Run as Administrator):
echo     certutil -addstore Root "%CERT_PUBLIC_FILE%"
echo.
echo   Option B — Via Windows GUI:
echo     1. Double-click %CERT_PUBLIC_FILE%
echo     2. Click "Install Certificate..."
echo     3. Select "Local Machine" ^> Next
echo     4. Select "Place all certificates in the following store"
echo     5. Browse ^> "Trusted Root Certification Authorities" ^> OK
echo     6. Next ^> Finish
echo.
echo   Option C — Via PowerShell (Run as Administrator):
echo     Import-Certificate -FilePath "%CERT_PUBLIC_FILE%" ^
echo         -CertStoreLocation "Cert:\LocalMachine\Root"
echo.
echo ═══ Important notes ═════════════════════════════════════════════
echo.
echo  • Self-signed certificates don't bypass SmartScreen for new users.
echo    They must install the .cer file first (one-time setup).
echo  • For automatic trust without user setup, use a commercial
echo    code signing certificate (DigiCert, Sectigo) or
echo    Microsoft Trusted Signing (~$10/month).
echo  • Open-source projects can apply for free signing at:
echo    https://www.signpath.io/foundation
echo.
pause
