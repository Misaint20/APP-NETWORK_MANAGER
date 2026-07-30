# APP-NETWORK_MANAGER — Installation Guide

## 📋 Table of Contents

1. [Quick Start](#-quick-start)
2. [Building from Source](#-building-from-source)
3. [Code Signing (Avoid SmartScreen)](#-code-signing-avoid-smartscreen)
4. [For End Users: Trust the Certificate](#-for-end-users-trust-the-certificate)
5. [Troubleshooting](#-troubleshooting)
6. [Commercial Signing (Advanced)](#-commercial-signing-advanced)

---

## 🚀 Quick Start

1. **Download** the latest `APP-NETWORK_MANAGER.exe` from the releases page
2. **Right-click** the file → **Properties**
3. If you see "This file came from another computer", check **Unblock** and click **OK**
4. **Right-click** → **Run as administrator**
5. If SmartScreen appears, click **More info** → **Run anyway**

> ⚠️ Windows may block the app because it's not digitally signed.
> See [Code Signing](#-code-signing-avoid-smartscreen) to fix this permanently.

---

## 🔧 Building from Source

### Requirements

| Tool | Version | Download |
|------|---------|----------|
| MinGW-w64 | GCC 12+ (C++20) | [mingw-w64.org](https://www.mingw-w64.org/) |
| Windows SDK | 10.0.x | Included with Visual Studio Build Tools |
| Resource compiler | windres | Included with MinGW-w64 |

### Building the GUI Version

```batch
cd UI
build.bat
```

This will produce `UI\APP-NETWORK_MANAGER.exe`.

### Building the Console Versions

```batch
:: Spanish version
g++ -std=c++20 app.cpp -o NETCONTROL.exe -lstdc++fs -lpthread

:: English version
g++ -std=c++20 app_en.cpp -o NETCONTROL_EN.exe -lstdc++fs -lpthread
```

---

## 🛡️ Code Signing (Avoid SmartScreen)

Windows SmartScreen blocks unknown executables. Code signing tells Windows
who created the file and that it hasn't been tampered with.

### Option 1: Self-Signed Certificate (Free)

This is the simplest option. The `.exe` gets a valid digital signature,
but users still need to trust your certificate once.

```batch
:: Run as Administrator
sign.bat
```

This will:
1. Generate a self-signed code signing certificate (valid for 5 years)
2. Sign `APP-NETWORK_MANAGER.exe` with SHA-256 + timestamp
3. Export `certificate-public.cer` for users to install

**What you get:**
```
UI/
├── APP-NETWORK_MANAGER.exe    ← Signed executable
├── certificate.pfx            ← Private key (keep secure!)
├── certificate-public.cer     ← Public key (share with users)
```

### Option 2: SignPath Foundation (Free for Open Source)

If your project is open-source, [SignPath Foundation](https://www.signpath.io/foundation)
signs your binaries **for free** using a real OV certificate.

Requirements:
- Public repository (GitHub/GitLab)
- Automated builds (GitHub Actions recommended)
- Project meets their eligibility criteria

### Option 3: Microsoft Trusted Signing (~$10/month)

[Azure Code Signing](https://learn.microsoft.com/en-us/azure/trusted-signing/)
is Microsoft's cloud-based signing service. Your app immediately shows
"Verified publisher" without reputation delays.

```
azuresign sign --file UI\APP-NETWORK_MANAGER.exe
```

### Option 4: Commercial Certificate ($200–500/year)

Buy an OV (Organization Validated) code signing certificate from:

| Provider | Approx. cost | Notes |
|----------|-------------|-------|
| DigiCert | $280/year | Widely trusted |
| Sectigo | $250/year | Good value |
| Certum | $200/year | Budget option |

With an OV certificate:
- ✅ Shows your verified company name
- ✅ No "Unknown publisher" warning
- ⏳ SmartScreen still needs reputation (download volume) for green checkmark

---

## 🔐 For End Users: Trust the Certificate

If you distributed a self-signed certificate (`certificate-public.cer`),
users must install it to avoid SmartScreen warnings.

### Option A: One Command (Fastest)

```batch
:: Run Command Prompt as Administrator
certutil -addstore Root "certificate-public.cer"
```

### Option B: Windows GUI

1. Double-click `certificate-public.cer`
2. Click **Install Certificate...**
3. Select **Local Machine** → **Next**
4. Select **Place all certificates in the following store**
5. Click **Browse...**
6. Select **Trusted Root Certification Authorities** → **OK**
7. Click **Next** → **Finish**
8. Click **Yes** on the security warning

### Option C: PowerShell

```powershell
# Run PowerShell as Administrator
Import-Certificate -FilePath "certificate-public.cer" -CertStoreLocation "Cert:\LocalMachine\Root"
```

### Option D: For IT Administrators (Group Policy)

Deploy the certificate to all domain computers via Group Policy:

1. Create a GPO for your domain
2. Go to: `Computer Configuration → Windows Settings → Security Settings →
   Public Key Policies → Trusted Root Certification Authorities`
3. Import the `.cer` file
4. Link the GPO to the target OUs

---

## 🔍 Troubleshooting

### "Windows protected your PC" / SmartScreen block

```
Windows protected your PC
Microsoft Defender SmartScreen prevented an unrecognized app from starting.
Running this app might put your PC at risk.
```

**Solution:**
1. Click **More info**
2. Click **Run anyway**
3. Or better: sign the file and install the certificate

### "Windows could not verify the publisher"

This means the file isn't signed, or the signing certificate isn't trusted.

**Solution:** Install the certificate (see [Trust the Certificate](#-for-end-users-trust-the-certificate))

### "The certificate is not trusted"

Self-signed certificates are not trusted by default. Users must install
the `.cer` file into their **Trusted Root Certification Authorities** store.

### "This file came from another computer and might be blocked"

Windows adds a Zone Identifier to files downloaded from the internet.

**Solution:**
1. Right-click the `.exe` → **Properties**
2. Check **Unblock** → **OK**

### signtool not found

If `sign.bat` can't find signtool.exe:

1. Install **Windows SDK** or **Visual Studio Build Tools**
2. Use **Developer Command Prompt for VS 2022**
3. Or add the SDK's bin folder to your PATH:
   ```batch
   set PATH=C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\;%PATH%
   ```

---

## 💼 Commercial Signing (Advanced)

### Why pay for a certificate?

| Feature | Self-Signed | Commercial OV | EV (discontinued) |
|---------|:-----------:|:-------------:|:-----------------:|
| Cost | Free | $200–500/yr | N/A |
| SmartScreen bypass | ❌ Need cert install | ✅ After reputation | N/A |
| Trusted by Windows | ❌ | ✅ | N/A |
| Shows publisher name | ❌ "Unknown" | ✅ Company name | N/A |

> Note: Microsoft removed the EV SmartScreen bypass in 2023. EV and OV
> certificates now behave identically for SmartScreen reputation.

### Using a Real Certificate

Once you have a commercial certificate:

```batch
:: Standard signing with timestamp
signtool sign /fd SHA256 /a "UI\APP-NETWORK_MANAGER.exe"

:: Sign with timestamp for long-term validity
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 "UI\APP-NETWORK_MANAGER.exe"

:: Verify the signature
signtool verify /v /pa "UI\APP-NETWORK_MANAGER.exe"
```

### Automating in CI/CD (GitHub Actions)

```yaml
- name: Sign executable
  run: |
    echo "$env:CERT_BASE64" | base64 -d > certificate.pfx
    signtool sign /f certificate.pfx /p "$env:CERT_PASSWORD" /fd SHA256 `
      /tr http://timestamp.digicert.com /td SHA256 UI\APP-NETWORK_MANAGER.exe
  env:
    CERT_BASE64: ${{ secrets.CERT_BASE64 }}
    CERT_PASSWORD: ${{ secrets.CERT_PASSWORD }}
```

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file.
