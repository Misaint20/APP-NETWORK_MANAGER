#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <shellapi.h>
#include <sddl.h>
using namespace std;
namespace fs = std::filesystem;

// Estructura para almacenar la información de la aplicación.
struct Application {
    string name;       // Nombre mostrado
    string folderPath; // Ruta de instalación (no se muestra al usuario)
};

// Función auxiliar para obtener un valor de tipo string del registro.
bool GetRegistryValueString(HKEY hKey, const string &valueName, string &valueOut) {
    DWORD type = 0;
    DWORD dataSize = 0;
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, nullptr, &dataSize) == ERROR_SUCCESS) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            vector<char> buffer(dataSize);
            if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(buffer.data()), &dataSize) == ERROR_SUCCESS) {
                valueOut = string(buffer.data());
                return true;
            }
        }
    }
    return false;
}

// Función que verifica si el programa se está ejecutando con privilegios de administrador.
bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

// Función para extraer la carpeta de una cadena que puede incluir el nombre de un ejecutable y, a veces, una coma (por ejemplo: "C:\Program Files\App\App.exe,0").
string ExtractFolderFromPath(const string &pathStr) {
    size_t commaPos = pathStr.find(',');
    string cleanPath = (commaPos != string::npos) ? pathStr.substr(0, commaPos) : pathStr;
    fs::path p(cleanPath);
    if (p.has_parent_path())
        return p.parent_path().string();
    return "";
}

// Función para enumerar una clave de desinstalación y agregar las aplicaciones encontradas.
void EnumerateUninstallKey(HKEY hKeyRoot, const string &subKey, vector<Application>& apps) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        char keyName[256];
        DWORD keyNameSize;
        DWORD index = 0;
        while (true) {
            keyNameSize = sizeof(keyName);
            LONG ret = RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr);
            if (ret == ERROR_NO_MORE_ITEMS)
                break;
            if (ret == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    string displayName;
                    if (GetRegistryValueString(hSubKey, "DisplayName", displayName)) {
                        Application app;
                        app.name = displayName;
                        // Primero se intenta obtener "InstallLocation"
                        if (!GetRegistryValueString(hSubKey, "InstallLocation", app.folderPath)) {
                            // Si no está, se intenta con "DisplayIcon" y se extrae la carpeta.
                            string displayIcon;
                            if (GetRegistryValueString(hSubKey, "DisplayIcon", displayIcon))
                                app.folderPath = ExtractFolderFromPath(displayIcon);
                        }
                        apps.push_back(app);
                    }
                    RegCloseKey(hSubKey);
                }
                index++;
            } else {
                break;
            }
        }
        RegCloseKey(hKey);
    }
}

// Modificación de main para aceptar parámetros y solicitar elevación si es necesario.
int main(int argc, char* argv[]) {
    if (!IsRunAsAdmin()) {
        char exePath[MAX_PATH];
        if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
            ShellExecuteA(NULL, "runas", exePath, GetCommandLineA(), NULL, SW_SHOWNORMAL);
        }
        return 0;
    }
    
    // Establecer el título de la ventana del programa.
    SetConsoleTitleA("NETCONTROL-APP - Gestor de Acceso a Internet");
    
    // Mostrar mensaje de carga.
    cout << "Cargando aplicaciones, por favor espere..." << endl;

    vector<Application> apps;
    // Se consultan varias claves del registro para obtener las aplicaciones instaladas.
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);
    EnumerateUninstallKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);

    // Eliminar duplicados (según el nombre) y ordenar.
    sort(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name < b.name;
    });
    apps.erase(unique(apps.begin(), apps.end(), [](const Application &a, const Application &b) {
        return a.name == b.name;
    }), apps.end());

    if (apps.empty()) {
        cout << "No se encontraron aplicaciones instaladas." << endl;
        system("pause");
        return 0;
    }

    while (true) { // Bucle principal para volver a la lista luego de ejecutar una acción.
        // --- Selección de la aplicación ---
        Application selectedApp;
        while (true) {
            cout << "\nAplicaciones instaladas:" << endl;
            for (size_t i = 0; i < apps.size(); i++) {
                cout << i + 1 << ". " << apps[i].name << endl;
            }
            cout << "0. Salir" << endl;
            cout << "\nSeleccione el numero de la aplicacion: ";
            int choice;
            cin >> choice;
            if (choice == 0) {
                cout << "Saliendo del programa." << endl;
                system("pause");
                return 0;
            }
            if (choice < 1 || choice > static_cast<int>(apps.size())) {
                cout << "Opcion invalida." << endl;
                continue;
            }
            selectedApp = apps[choice - 1];
            cout << "Ha seleccionado: " << selectedApp.name << endl;
            cout << "\nSeleccione:" << endl;
            cout << "1. Continuar con esta aplicacion" << endl;
            cout << "2. Regresar a la lista de aplicaciones" << endl;
            int confirm;
            cin >> confirm;
            if (confirm == 1) {
                break;
            }
            // Si se elige 2, se regresa a la lista.
        }
        
        // Si no se obtuvo la ruta de instalación, solicitarla manualmente.
        if (selectedApp.folderPath.empty()) {
            cout << "No se encontro la ubicación de instalacion para esta aplicacion." << endl;
            cout << "Ingrese manualmente la ruta de la carpeta: ";
            cin.ignore();
            getline(cin, selectedApp.folderPath);
        }
        
        // --- Selección del ejecutable ---
        vector<string> exeFiles;
        try {
            for (const auto &entry : fs::directory_iterator(selectedApp.folderPath)) {
                if (entry.is_regular_file()) {
                    fs::path p = entry.path();
                    if (p.extension() == ".exe" || p.extension() == ".EXE")
                        exeFiles.push_back(p.string());
                }
            }
        } catch (...) {
            // Si ocurre algún error al acceder a la carpeta.
        }
        
        string targetExe;
        if (exeFiles.empty()) {
            cout << "No se encontraron archivos .exe en la carpeta." << endl;
            cout << "Ingrese manualmente la ruta completa del ejecutable: ";
            cin.ignore();
            getline(cin, targetExe);
        } else if (exeFiles.size() == 1) {
            targetExe = exeFiles[0];
            cout << "Se ha detectado un unico ejecutable en la carpeta." << endl;
        } else {
            cout << "Se encontraron múltiples ejecutables en la carpeta:" << endl;
            for (size_t i = 0; i < exeFiles.size(); i++) {
                cout << i + 1 << ". " << exeFiles[i] << endl;
            }
            cout << "Seleccione el numero del ejecutable que desea usar: ";
            int exeChoice;
            cin >> exeChoice;
            if (exeChoice < 1 || exeChoice > static_cast<int>(exeFiles.size())) {
                cout << "Opción invalida. Se usará el primer ejecutable encontrado." << endl;
                targetExe = exeFiles[0];
            } else {
                targetExe = exeFiles[exeChoice - 1];
            }
        }
        
        cout << "\nRuta del ejecutable seleccionado: " << targetExe << endl;
        
        // --- Selección de la accion ---
        int action = 0;
        while (true) {
            cout << "\nSeleccione la accion:" << endl;
            cout << "1. Bloquear acceso a Internet" << endl;
            cout << "2. Permitir acceso a Internet" << endl;
            cout << "3. Regresar a la lista de aplicaciones" << endl;
            cout << "Ingrese su opcion: ";
            cin >> action;
            if (action == 1 || action == 2 || action == 3) {
                break;
            }
            cout << "Opcion invalida. Por favor intente de nuevo." << endl;
        }
        
        if (action == 3) {
            continue; // Regresa a la lista de aplicaciones.
        }
        
        // Se define un nombre para la regla del Firewall basado en el nombre de la aplicación.
        string ruleName = "Bloqueo_" + selectedApp.name;
        string command;
        if (action == 1) {
            command = "netsh advfirewall firewall add rule name=\"" + ruleName +
                      "\" dir=out action=block program=\"" + targetExe + "\" enable=yes";
        } else if (action == 2) {
            command = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\"";
        }
        
        cout << "\nEjecutando comando: " << command << endl;
        system(command.c_str());
        
        cout << "\nAccion ejecutada. Presione cualquier tecla para volver a la lista de aplicaciones." << endl;
        system("pause");
    }
}