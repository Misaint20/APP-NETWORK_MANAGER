#include "LanguageManager.h"
#include <vector>
#include <algorithm>

// ── Singleton ────────────────────────────────────────────────────────────

LanguageManager& LanguageManager::getInstance() {
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager() {
    m_lang = detectSystemLanguage();
}

// ── Auto-detect from Windows OS locale ───────────────────────────────────

Language LanguageManager::detectSystemLanguage() {
    LANGID langId = GetUserDefaultUILanguage();
    switch(PRIMARYLANGID(langId)) {
        case LANG_SPANISH:   return Language::Spanish;
        case LANG_FRENCH:    return Language::French;
        case LANG_GERMAN:    return Language::German;
        case LANG_ITALIAN:   return Language::Italian;
        case LANG_PORTUGUESE:return Language::Portuguese;
        case LANG_DUTCH:     return Language::Dutch;
        case LANG_RUSSIAN:   return Language::Russian;
        case LANG_CHINESE:   return Language::Chinese;
        case LANG_JAPANESE:  return Language::Japanese;
        case LANG_KOREAN:    return Language::Korean;
        case LANG_ARABIC:    return Language::Arabic;
        case LANG_TURKISH:   return Language::Turkish;
        case LANG_POLISH:    return Language::Polish;
        case LANG_SWEDISH:   return Language::Swedish;
        default:             return Language::English;
    }
}

// ── Language display names (native names) ────────────────────────────────

const wchar_t* LanguageManager::getLanguageName(Language lang) {
    static const wchar_t* names[] = {
        L"English",
        L"Espa\u00F1ol",           // Spanish
        L"Fran\u00E7ais",          // French
        L"Deutsch",                // German
        L"Italiano",               // Italian
        L"Portugu\u00EAs",         // Portuguese
        L"Nederlands",             // Dutch
        L"\u0420\u0443\u0441\u0441\u043A\u0438\u0439",  // Russian
        L"\u4E2D\u6587",           // Chinese (中文)
        L"\u65E5\u672C\u8A9E",     // Japanese (日本語)
        L"\uD55C\uAD6D\uC5B4",     // Korean (한국어)
        L"\u0627\u0644\u0639\u0631\u0628\u064A\u0629", // Arabic (العربية)
        L"T\u00FCrk\u00E7e",       // Turkish
        L"Polski",                 // Polish
        L"Svenska"                 // Swedish
    };
    return names[static_cast<int>(lang)];
}

// ── Helper: replace {1} and {2} in format strings ────────────────────────

static std::wstring ReplaceArgs(const std::wstring& fmt,
                                 const std::wstring& a1 = L"",
                                 const std::wstring& a2 = L"") {
    std::wstring r = fmt;
    size_t pos;
    pos = r.find(L"{1}"); if(pos != std::wstring::npos) r.replace(pos, 3, a1);
    pos = r.find(L"{2}"); if(pos != std::wstring::npos) r.replace(pos, 3, a2);
    return r;
}

// ── Translation tables ───────────────────────────────────────────────────
//  Rows = languages in Language enum order
//  Cols = StringId entries

#define _ static_cast<int>(StringId::Count)

const LanguageManager::Table LanguageManager::s_translations[] = {

    // ────────── English (default) ─────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Internet Access Controller",
        L"\U0001F504 Refresh",
        L"Installed Applications ({1} of {2})",
        L"Working...",
        L"Application Name",
        L"Status",
        L"\U0001F6AB Blocked",
        L"\u2705 Allowed",
        L"\U0001F6AB Block Internet",
        L"\u2705 Allow Internet",
        L"Exit",
        L"Scanning installed applications...",
        L"Scanning installed applications and firewall rules...",
        L"Ready \u2014 {1} applications loaded.",
        L"Blocking Internet access for {1}...",
        L"Allowing Internet access for {1}...",
        L"\u2713  Internet access blocked successfully.",
        L"\u2713  Internet access allowed successfully.",
        L"\u2717  Failed to block Internet access. Try running as Administrator.",
        L"\u2717  Failed to allow Internet access. Try running as Administrator.",
        L"Please select an application first.",
        L"Installation path not found for this application.",
        L"No executables found for this application.",
        L"Internet access blocked successfully.\n\nThe firewall rule has been created.",
        L"Internet access allowed successfully.\n\nThe firewall rule has been removed.",
        L"Failed to block Internet access.\n\nMake sure the application is running with Administrator privileges.",
        L"Failed to allow Internet access.\n\nMake sure the application is running with Administrator privileges.",
        L"Information",
        L"Error",
        L"Success",
        L"{1} of {2} applications currently blocked",
        L"Unblock All",
        L"No applications match your search"
    },

    // ────────── Spanish ──────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Control de Acceso a Internet",
        L"\U0001F504 Actualizar",
        L"Aplicaciones instaladas ({1} de {2})",
        L"Trabajando...",
        L"Nombre de la aplicaci\u00F3n",
        L"Estado",
        L"\U0001F6AB Bloqueado",
        L"\u2705 Permitido",
        L"\U0001F6AB Bloquear Internet",
        L"\u2705 Permitir Internet",
        L"Salir",
        L"Escaneando aplicaciones instaladas...",
        L"Escaneando aplicaciones instaladas y reglas de firewall...",
        L"Listo \u2014 {1} aplicaciones cargadas.",
        L"Bloqueando acceso a Internet para {1}...",
        L"Permitiendo acceso a Internet para {1}...",
        L"\u2713  Acceso a Internet bloqueado exitosamente.",
        L"\u2713  Acceso a Internet permitido exitosamente.",
        L"\u2717  Error al bloquear el acceso. Ejecute como Administrador.",
        L"\u2717  Error al permitir el acceso. Ejecute como Administrador.",
        L"Por favor seleccione una aplicaci\u00F3n primero.",
        L"No se encontr\u00F3 la ruta de instalaci\u00F3n.",
        L"No se encontraron ejecutables para esta aplicaci\u00F3n.",
        L"Acceso a Internet bloqueado exitosamente.\n\nLa regla de firewall ha sido creada.",
        L"Acceso a Internet permitido exitosamente.\n\nLa regla de firewall ha sido eliminada.",
        L"Error al bloquear el acceso a Internet.\n\nAseg\u00FArese de ejecutar como Administrador.",
        L"Error al permitir el acceso a Internet.\n\nAseg\u00FArese de ejecutar como Administrador.",
        L"Informaci\u00F3n",
        L"Error",
        L"\u00C9xito",
        L"{1} de {2} aplicaciones actualmente bloqueadas",
        L"Desbloquear Todo",
        L"Sin resultados"
    },

    // ────────── French ───────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Contr\u00F4le d'Acc\u00E8s Internet",
        L"\U0001F504 Actualiser",
        L"Applications install\u00E9es ({1} sur {2})",
        L"En cours...",
        L"Nom de l'application",
        L"\u00C9tat",
        L"\U0001F6AB Bloqu\u00E9",
        L"\u2705 Autoris\u00E9",
        L"\U0001F6AB Bloquer Internet",
        L"\u2705 Autoriser Internet",
        L"Quitter",
        L"Analyse des applications install\u00E9es...",
        L"Analyse des applications et des r\u00E8gles pare-feu...",
        L"Pr\u00EAt \u2014 {1} applications charg\u00E9es.",
        L"Blocage de l'acc\u00E8s Internet pour {1}...",
        L"Autorisation de l'acc\u00E8s Internet pour {1}...",
        L"\u2713  Acc\u00E8s Internet bloqu\u00E9 avec succ\u00E8s.",
        L"\u2713  Acc\u00E8s Internet autoris\u00E9 avec succ\u00E8s.",
        L"\u2717  \u00C9chec du blocage. Ex\u00E9cutez en tant qu'Administrateur.",
        L"\u2717  \u00C9chec de l'autorisation. Ex\u00E9cutez en tant qu'Administrateur.",
        L"Veuillez d'abord s\u00E9lectionner une application.",
        L"Chemin d'installation introuvable.",
        L"Aucun ex\u00E9cutable trouv\u00E9.",
        L"Acc\u00E8s Internet bloqu\u00E9 avec succ\u00E8s.\n\nLa r\u00E8gle pare-feu a \u00E9t\u00E9 cr\u00E9\u00E9e.",
        L"Acc\u00E8s Internet autoris\u00E9 avec succ\u00E8s.\n\nLa r\u00E8gle pare-feu a \u00E9t\u00E9 supprim\u00E9e.",
        L"\u00C9chec du blocage de l'acc\u00E8s Internet.\n\nEx\u00E9cutez en tant qu'Administrateur.",
        L"\u00C9chec de l'autorisation de l'acc\u00E8s Internet.\n\nEx\u00E9cutez en tant qu'Administrateur.",
        L"Information",
        L"Erreur",
        L"Succ\u00E8s",
        L"{1} sur {2} applications actuellement bloqu\u00E9es",
        L"D\u00E9bloquer Tout",
        L"Aucun r\u00E9sultat"
    },

    // ────────── German ────────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Internetzugangssteuerung",
        L"\U0001F504 Aktualisieren",
        L"Installierte Anwendungen ({1} von {2})",
        L"Arbeite...",
        L"Anwendungsname",
        L"Status",
        L"\U0001F6AB Blockiert",
        L"\u2705 Erlaubt",
        L"\U0001F6AB Internet blockieren",
        L"\u2705 Internet erlauben",
        L"Beenden",
        L"Installierte Anwendungen werden gescannt...",
        L"Anwendungen und Firewallregeln werden gescannt...",
        L"Bereit \u2014 {1} Anwendungen geladen.",
        L"Internetzugriff wird blockiert f\u00FCr {1}...",
        L"Internetzugriff wird erlaubt f\u00FCr {1}...",
        L"\u2713  Internetzugriff erfolgreich blockiert.",
        L"\u2713  Internetzugriff erfolgreich erlaubt.",
        L"\u2717  Blockierung fehlgeschlagen. Als Administrator ausf\u00FChren.",
        L"\u2717  Freigabe fehlgeschlagen. Als Administrator ausf\u00FChren.",
        L"Bitte w\u00E4hlen Sie zuerst eine Anwendung aus.",
        L"Installationspfad nicht gefunden.",
        L"Keine ausf\u00FChrbaren Dateien gefunden.",
        L"Internetzugriff erfolgreich blockiert.\n\nDie Firewallregel wurde erstellt.",
        L"Internetzugriff erfolgreich erlaubt.\n\nDie Firewallregel wurde entfernt.",
        L"Blockierung des Internetzugriffs fehlgeschlagen.\n\nAls Administrator ausf\u00FChren.",
        L"Freigabe des Internetzugriffs fehlgeschlagen.\n\nAls Administrator ausf\u00FChren.",
        L"Information",
        L"Fehler",
        L"Erfolg",
        L"{1} von {2} Anwendungen derzeit blockiert",
        L"Alle Entsperren",
        L"Keine Ergebnisse"
    },

    // ────────── Italian ───────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Controllo Accesso Internet",
        L"\U0001F504 Aggiorna",
        L"Applicazioni installate ({1} di {2})",
        L"In esecuzione...",
        L"Nome applicazione",
        L"Stato",
        L"\U0001F6AB Bloccato",
        L"\u2705 Consentito",
        L"\U0001F6AB Blocca Internet",
        L"\u2705 Consenti Internet",
        L"Esci",
        L"Scansione applicazioni installate...",
        L"Scansione applicazioni e regole firewall...",
        L"Pronto \u2014 {1} applicazioni caricate.",
        L"Blocco dell'accesso a Internet per {1}...",
        L"Consentimento dell'accesso a Internet per {1}...",
        L"\u2713  Accesso a Internet bloccato con successo.",
        L"\u2713  Accesso a Internet consentito con successo.",
        L"\u2717  Blocco fallito. Eseguire come Amministratore.",
        L"\u2717  Autorizzazione fallita. Eseguire come Amministratore.",
        L"Selezionare prima un'applicazione.",
        L"Percorso di installazione non trovato.",
        L"Nessun eseguibile trovato.",
        L"Accesso a Internet bloccato con successo.\n\nLa regola firewall \u00E8 stata creata.",
        L"Accesso a Internet consentito con successo.\n\nLa regola firewall \u00E8 stata rimossa.",
        L"Blocco dell'accesso a Internet fallito.\n\nEseguire come Amministratore.",
        L"Autorizzazione dell'accesso a Internet fallita.\n\nEseguire come Amministratore.",
        L"Informazioni",
        L"Errore",
        L"Successo",
        L"{1} di {2} applicazioni attualmente bloccate",
        L"Sblocca Tutto",
        L"Nessun risultato"
    },

    // ────────── Portuguese ────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Controlador de Acesso \u00E0 Internet",
        L"\U0001F504 Atualizar",
        L"Aplicativos instalados ({1} de {2})",
        L"Trabalhando...",
        L"Nome do aplicativo",
        L"Status",
        L"\U0001F6AB Bloqueado",
        L"\u2705 Permitido",
        L"\U0001F6AB Bloquear Internet",
        L"\u2705 Permitir Internet",
        L"Sair",
        L"Escaneando aplicativos instalados...",
        L"Escaneando aplicativos e regras de firewall...",
        L"Pronto \u2014 {1} aplicativos carregados.",
        L"Bloqueando acesso \u00E0 Internet para {1}...",
        L"Permitindo acesso \u00E0 Internet para {1}...",
        L"\u2713  Acesso \u00E0 Internet bloqueado com sucesso.",
        L"\u2713  Acesso \u00E0 Internet permitido com sucesso.",
        L"\u2717  Falha ao bloquear. Execute como Administrador.",
        L"\u2717  Falha ao permitir. Execute como Administrador.",
        L"Selecione um aplicativo primeiro.",
        L"Caminho de instala\u00E7\u00E3o n\u00E3o encontrado.",
        L"Nenhum execut\u00E1vel encontrado.",
        L"Acesso \u00E0 Internet bloqueado com sucesso.\n\nA regra de firewall foi criada.",
        L"Acesso \u00E0 Internet permitido com sucesso.\n\nA regra de firewall foi removida.",
        L"Falha ao bloquear acesso \u00E0 Internet.\n\nExecute como Administrador.",
        L"Falha ao permitir acesso \u00E0 Internet.\n\nExecute como Administrador.",
        L"Informa\u00E7\u00E3o",
        L"Erro",
        L"Sucesso",
        L"{1} de {2} aplicativos atualmente bloqueados",
        L"Desbloquear Tudo",
        L"Sem resultados"
    },

    // ────────── Dutch ─────────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Internettoegangsbeheer",
        L"\U0001F504 Verversen",
        L"Ge\u00EFnstalleerde apps ({1} van {2})",
        L"Bezig...",
        L"App-naam",
        L"Status",
        L"\U0001F6AB Geblokkeerd",
        L"\u2705 Toegestaan",
        L"\U0001F6AB Internet blokkeren",
        L"\u2705 Internet toestaan",
        L"Afsluiten",
        L"Ge\u00EFnstalleerde apps scannen...",
        L"Apps en firewallregels scannen...",
        L"Gereed \u2014 {1} apps geladen.",
        L"Internettoegang blokkeren voor {1}...",
        L"Internettoegang toestaan voor {1}...",
        L"\u2713  Internettoegang succesvol geblokkeerd.",
        L"\u2713  Internettoegang succesvol toegestaan.",
        L"\u2717  Blokkeren mislukt. Uitvoeren als Administrator.",
        L"\u2717  Toestaan mislukt. Uitvoeren als Administrator.",
        L"Selecteer eerst een applicatie.",
        L"Installatiepad niet gevonden.",
        L"Geen uitvoerbare bestanden gevonden.",
        L"Internettoegang succesvol geblokkeerd.\n\nDe firewallregel is aangemaakt.",
        L"Internettoegang succesvol toegestaan.\n\nDe firewallregel is verwijderd.",
        L"Blokkeren van internettoegang mislukt.\n\nUitvoeren als Administrator.",
        L"Toestaan van internettoegang mislukt.\n\nUitvoeren als Administrator.",
        L"Informatie",
        L"Fout",
        L"Succes",
        L"{1} van {2} apps momenteel geblokkeerd",
        L"Alles Deblokkeren",
        L"Geen resultaten"
    },

    // ────────── Russian ───────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \u0423\u043F\u0440\u0430\u0432\u043B\u0435\u043D\u0438\u0435 \u0434\u043E\u0441\u0442\u0443\u043F\u043E\u043C \u043A \u0418\u043D\u0442\u0435\u0440\u043D\u0435\u0442\u0443",
        L"\U0001F504 \u041E\u0431\u043D\u043E\u0432\u0438\u0442\u044C",
        L"\u0423\u0441\u0442\u0430\u043D\u043E\u0432\u043B\u0435\u043D\u043D\u044B\u0435 \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u044F ({1} \u0438\u0437 {2})",
        L"\u041F\u043E\u0434\u043E\u0436\u0434\u0438\u0442\u0435...",
        L"\u0418\u043C\u044F \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u044F",
        L"\u0421\u0442\u0430\u0442\u0443\u0441",
        L"\U0001F6AB \u0417\u0430\u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u043D\u043E",
        L"\u2705 \u0420\u0430\u0437\u0440\u0435\u0448\u0435\u043D\u043E",
        L"\U0001F6AB \u0411\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u0442\u044C \u0418\u043D\u0442\u0435\u0440\u043D\u0435\u0442",
        L"\u2705 \u0420\u0430\u0437\u0440\u0435\u0448\u0438\u0442\u044C \u0418\u043D\u0442\u0435\u0440\u043D\u0435\u0442",
        L"\u0412\u044B\u0445\u043E\u0434",
        L"\u0421\u043A\u0430\u043D\u0438\u0440\u043E\u0432\u0430\u043D\u0438\u0435 \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u0439...",
        L"\u0421\u043A\u0430\u043D\u0438\u0440\u043E\u0432\u0430\u043D\u0438\u0435 \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u0439 \u0438 \u043F\u0440\u0430\u0432\u0438\u043B \u0444\u0430\u0435\u0440\u0432\u043E\u043B\u0430...",
        L"\u0413\u043E\u0442\u043E\u0432\u043E \u2014 \u0437\u0430\u0433\u0440\u0443\u0436\u0435\u043D\u043E {1} \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u0439.",
        L"\u0411\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u043A\u0430 \u0434\u043E\u0441\u0442\u0443\u043F\u0430 \u043A \u0418\u043D\u0442\u0435\u0440\u043D\u0435\u0442\u0443 \u0434\u043B\u044F {1}...",
        L"\u0420\u0430\u0437\u0440\u0435\u0448\u0435\u043D\u0438\u0435 \u0434\u043E\u0441\u0442\u0443\u043F\u0430 \u043A \u0418\u043D\u0442\u0435\u0440\u043D\u0435\u0442\u0443 \u0434\u043B\u044F {1}...",
        L"\u2713  \u0414\u043E\u0441\u0442\u0443\u043F \u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u043D \u0443\u0441\u043F\u0435\u0448\u043D\u043E.",
        L"\u2713  \u0414\u043E\u0441\u0442\u0443\u043F \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043D \u0443\u0441\u043F\u0435\u0448\u043D\u043E.",
        L"\u2717  \u041E\u0448\u0438\u0431\u043A\u0430 \u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u043A\u0438. \u0417\u0430\u043F\u0443\u0441\u0442\u0438\u0442\u0435 \u043A\u0430\u043A \u0410\u0434\u043C\u0438\u043D\u0438\u0441\u0442\u0440\u0430\u0442\u043E\u0440.",
        L"\u2717  \u041E\u0448\u0438\u0431\u043A\u0430 \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043D\u0438\u044F. \u0417\u0430\u043F\u0443\u0441\u0442\u0438\u0442\u0435 \u043A\u0430\u043A \u0410\u0434\u043C\u0438\u043D\u0438\u0441\u0442\u0440\u0430\u0442\u043E\u0440.",
        L"\u041F\u043E\u0436\u0430\u043B\u0443\u0439\u0441\u0442\u0430, \u0432\u044B\u0431\u0435\u0440\u0438\u0442\u0435 \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u0435.",
        L"\u041F\u0443\u0442\u044C \u0443\u0441\u0442\u0430\u043D\u043E\u0432\u043A\u0438 \u043D\u0435 \u043D\u0430\u0439\u0434\u0435\u043D.",
        L"\u0418\u0441\u043F\u043E\u043B\u043D\u044F\u0435\u043C\u044B\u0435 \u0444\u0430\u0439\u043B\u044B \u043D\u0435 \u043D\u0430\u0439\u0434\u0435\u043D\u044B.",
        L"\u0414\u043E\u0441\u0442\u0443\u043F \u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u043D.\n\n\u041F\u0440\u0430\u0432\u0438\u043B\u043E \u0444\u0430\u0435\u0440\u0432\u043E\u043B\u0430 \u0441\u043E\u0437\u0434\u0430\u043D\u043E.",
        L"\u0414\u043E\u0441\u0442\u0443\u043F \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043D.\n\n\u041F\u0440\u0430\u0432\u0438\u043B\u043E \u0444\u0430\u0435\u0440\u0432\u043E\u043B\u0430 \u0443\u0434\u0430\u043B\u0435\u043D\u043E.",
        L"\u041E\u0448\u0438\u0431\u043A\u0430 \u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u043A\u0438.\n\n\u0417\u0430\u043F\u0443\u0441\u0442\u0438\u0442\u0435 \u043A\u0430\u043A \u0410\u0434\u043C\u0438\u043D\u0438\u0441\u0442\u0440\u0430\u0442\u043E\u0440.",
        L"\u041E\u0448\u0438\u0431\u043A\u0430 \u0440\u0430\u0437\u0440\u0435\u0448\u0435\u043D\u0438\u044F.\n\n\u0417\u0430\u043F\u0443\u0441\u0442\u0438\u0442\u0435 \u043A\u0430\u043A \u0410\u0434\u043C\u0438\u043D\u0438\u0441\u0442\u0440\u0430\u0442\u043E\u0440.",
        L"\u0418\u043D\u0444\u043E\u0440\u043C\u0430\u0446\u0438\u044F",
        L"\u041E\u0448\u0438\u0431\u043A\u0430",
        L"\u0423\u0441\u043F\u0435\u0445",
        L"\u0422\u0435\u043A\u0443\u0449\u0435 \u0437\u0430\u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u043D\u043E: {1} \u0438\u0437 {2} \u043F\u0440\u0438\u043B\u043E\u0436\u0435\u043D\u0438\u0439",
        L"\u0420\u0430\u0437\u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u0442\u044C \u0412\u0441\u0451",
        L"\u041D\u0435\u0442 \u0440\u0435\u0437\u0443\u043B\u044C\u0442\u0430\u0442\u043E\u0432"
    },

    // ────────── Chinese Simplified ────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \u4E92\u8054\u7F51\u8BBF\u95EE\u63A7\u5236\u5668",
        L"\U0001F504 \u5237\u65B0",
        L"\u5DF2\u5B89\u88C5\u5E94\u7528\u7A0B\u5E8F ({1}/{2})",
        L"\u6B63\u5728\u5DE5\u4F5C...",
        L"\u5E94\u7528\u7A0B\u5E8F\u540D\u79F0",
        L"\u72B6\u6001",
        L"\U0001F6AB \u5DF2\u62A2\u6B65",
        L"\u2705 \u5DF2\u5141\u8BB8",
        L"\U0001F6AB \u62A2\u6B65\u4E92\u8054\u7F51",
        L"\u2705 \u5141\u8BB8\u4E92\u8054\u7F51",
        L"\u9000\u51FA",
        L"\u626B\u63CF\u5DF2\u5B89\u88C5\u7684\u5E94\u7528\u7A0B\u5E8F...",
        L"\u626B\u63CF\u5E94\u7528\u7A0B\u5E8F\u548C\u9632\u706B\u5899\u89C4\u5219...",
        L"\u5C31\u7EEA \u2014 \u5DF2\u52A0\u8F7D {1} \u4E2A\u5E94\u7528\u7A0B\u5E8F\u3002",
        L"\u6B63\u5728\u62A2\u6B65 {1} \u7684\u4E92\u8054\u7F51\u8BBF\u95EE...",
        L"\u6B63\u5728\u5141\u8BB8 {1} \u7684\u4E92\u8054\u7F51\u8BBF\u95EE...",
        L"\u2713  \u4E92\u8054\u7F51\u8BBF\u95EE\u5DF2\u6210\u529F\u62A2\u6B65\u3002",
        L"\u2713  \u4E92\u8054\u7F51\u8BBF\u95EE\u5DF2\u6210\u529F\u5141\u8BB8\u3002",
        L"\u2717  \u62A2\u6B65\u5931\u8D25\u3002\u8BF7\u4EE5\u7BA1\u7406\u5458\u8EAB\u4EFD\u8FD0\u884C\u3002",
        L"\u2717  \u5141\u8BB8\u5931\u8D25\u3002\u8BF7\u4EE5\u7BA1\u7406\u5458\u8EAB\u4EFD\u8FD0\u884C\u3002",
        L"\u8BF7\u5148\u9009\u62E9\u4E00\u4E2A\u5E94\u7528\u7A0B\u5E8F\u3002",
        L"\u672A\u627E\u5230\u5B89\u88C5\u8DEF\u5F84\u3002",
        L"\u672A\u627E\u5230\u53EF\u6267\u884C\u6587\u4EF6\u3002",
        L"\u4E92\u8054\u7F51\u8BBF\u95EE\u5DF2\u6210\u529F\u62A2\u6B65\u3002\n\n\u9632\u706B\u5899\u89C4\u5219\u5DF2\u521B\u5EFA\u3002",
        L"\u4E92\u8054\u7F51\u8BBF\u95EE\u5DF2\u6210\u529F\u5141\u8BB8\u3002\n\n\u9632\u706B\u5899\u89C4\u5219\u5DF2\u5220\u9664\u3002",
        L"\u62A2\u6B65\u4E92\u8054\u7F51\u8BBF\u95EE\u5931\u8D25\u3002\n\n\u8BF7\u4EE5\u7BA1\u7406\u5458\u8EAB\u4EFD\u8FD0\u884C\u3002",
        L"\u5141\u8BB8\u4E92\u8054\u7F51\u8BBF\u95EE\u5931\u8D25\u3002\n\n\u8BF7\u4EE5\u7BA1\u7406\u5458\u8EAB\u4EFD\u8FD0\u884C\u3002",
        L"\u4FE1\u606F",
        L"\u9519\u8BEF",
        L"\u6210\u529F",
        L"\u5F53\u524D\u62A2\u6B65\u4E86 {1}/{2} \u4E2A\u5E94\u7528\u7A0B\u5E8F",
        L"\u89E3\u9664\u6240\u6709\u62A2\u6B65",
        L"\u65E0\u7ED3\u679C"
    },

    // ────────── Japanese ──────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u7BA1\u7406",
        L"\U0001F504 \u66F4\u65B0",
        L"\u30A4\u30F3\u30B9\u30C8\u30FC\u30EB\u6E08\u30A2\u30D7\u30EA ({1}/{2})",
        L"\u4F5C\u696D\u4E2D...",
        L"\u30A2\u30D7\u30EA\u30B1\u30FC\u30B7\u30E7\u30F3\u540D",
        L"\u30B9\u30C6\u30FC\u30BF\u30B9",
        L"\U0001F6AB \u30D6\u30ED\u30C3\u30AF\u4E2D",
        L"\u2705 \u8A31\u53EF\u6E08",
        L"\U0001F6AB \u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u3092\u30D6\u30ED\u30C3\u30AF",
        L"\u2705 \u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u3092\u8A31\u53EF",
        L"\u7D42\u4E86",
        L"\u30A4\u30F3\u30B9\u30C8\u30FC\u30EB\u6E08\u30A2\u30D7\u30EA\u3092\u30B9\u30AD\u30E3\u30F3\u4E2D...",
        L"\u30A2\u30D7\u30EA\u3068\u30D5\u30A1\u30A4\u30A2\u30A6\u30A9\u30FC\u30EB\u898F\u5247\u3092\u30B9\u30AD\u30E3\u30F3\u4E2D...",
        L"\u6E96\u5099\u5B8C\u4E86 \u2014 {1} \u306E\u30A2\u30D7\u30EA\u3092\u8AAD\u307F\u8FBC\u307F\u307E\u3057\u305F\u3002",
        L"{1} \u3078\u306E\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u30D6\u30ED\u30C3\u30AF\u4E2D...",
        L"{1} \u3078\u306E\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u8A31\u53EF\u4E2D...",
        L"\u2713  \u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u30D6\u30ED\u30C3\u30AF\u3057\u307E\u3057\u305F\u3002",
        L"\u2713  \u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u8A31\u53EF\u3057\u307E\u3057\u305F\u3002",
        L"\u2717  \u30D6\u30ED\u30C3\u30AF\u306B\u5931\u6557\u3057\u307E\u3057\u305F\u3002\u7BA1\u7406\u8005\u3068\u3057\u3066\u5B9F\u884C\u3057\u3066\u304F\u3060\u3055\u3044\u3002",
        L"\u2717  \u8A31\u53EF\u306B\u5931\u6557\u3057\u307E\u3057\u305F\u3002\u7BA1\u7406\u8005\u3068\u3057\u3066\u5B9F\u884C\u3057\u3066\u304F\u3060\u3055\u3044\u3002",
        L"\u307E\u305A\u30A2\u30D7\u30EA\u30B1\u30FC\u30B7\u30E7\u30F3\u3092\u9078\u629E\u3057\u3066\u304F\u3060\u3055\u3044\u3002",
        L"\u30A4\u30F3\u30B9\u30C8\u30FC\u30EB\u30D1\u30B9\u304C\u898B\u3064\u304B\u308A\u307E\u305B\u3093\u3002",
        L"\u5B9F\u884C\u30D5\u30A1\u30A4\u30EB\u304C\u898B\u3064\u304B\u308A\u307E\u305B\u3093\u3002",
        L"\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u30D6\u30ED\u30C3\u30AF\u3057\u307E\u3057\u305F\u3002\n\n\u30D5\u30A1\u30A4\u30A2\u30A6\u30A9\u30FC\u30EB\u898F\u5247\u304C\u4F5C\u6210\u3055\u308C\u307E\u3057\u305F\u3002",
        L"\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u3092\u8A31\u53EF\u3057\u307E\u3057\u305F\u3002\n\n\u30D5\u30A1\u30A4\u30A2\u30A6\u30A9\u30FC\u30EB\u898F\u5247\u304C\u524A\u9664\u3055\u308C\u307E\u3057\u305F\u3002",
        L"\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u306E\u30D6\u30ED\u30C3\u30AF\u306B\u5931\u6557\u3057\u307E\u3057\u305F\u3002\n\n\u7BA1\u7406\u8005\u3068\u3057\u3066\u5B9F\u884C\u3057\u3066\u304F\u3060\u3055\u3044\u3002",
        L"\u30A4\u30F3\u30BF\u30FC\u30CD\u30C3\u30C8\u30A2\u30AF\u30BB\u30B9\u306E\u8A31\u53EF\u306B\u5931\u6557\u3057\u307E\u3057\u305F\u3002\n\n\u7BA1\u7406\u8005\u3068\u3057\u3066\u5B9F\u884C\u3057\u3066\u304F\u3060\u3055\u3044\u3002",
        L"\u60C5\u5831",
        L"\u30A8\u30E9\u30FC",
        L"\u6210\u529F",
        L"\u73FE\u5728 {1}/{2} \u306E\u30A2\u30D7\u30EA\u304C\u30D6\u30ED\u30C3\u30AF\u4E2D",
        L"\u3059\u3079\u3066\u89E3\u9664",
        L"\u7D50\u679C\u306A\u3057"
    },

    // ────────── Korean ────────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \uC778\uD130\uB137 \uC561\uC138\uC2A4 \uCEE8\uD2B8\uB864\uB7EC",
        L"\U0001F504 \uC0C8\uB85C\uACE0\uCE68",
        L"\uC124\uCE58\uB41C \uC560\uD50C\uB9AC\uCF00\uC774\uC158 ({1}/{2})",
        L"\uC791\uC5C5 \uC911...",
        L"\uC560\uD50C\uB9AC\uCF00\uC774\uC158 \uC774\uB984",
        L"\uC0C1\uD0DC",
        L"\U0001F6AB \uCC28\uB2E8\uB428",
        L"\u2705 \uD5C8\uC6A9\uB428",
        L"\U0001F6AB \uC778\uD130\uB137 \uCC28\uB2E8",
        L"\u2705 \uC778\uD130\uB137 \uD5C8\uC6A9",
        L"\uC885\uB8CC",
        L"\uC124\uCE58\uB41C \uC560\uD50C\uB9AC\uCF00\uC774\uC158 \uC2A4\uCE94 \uC911...",
        L"\uC560\uD50C\uB9AC\uCF00\uC774\uC158 \uBC0F \uBC29\uD654\uBCBD \uADDC\uCE59 \uC2A4\uCE94 \uC911...",
        L"\uC900\uBE44 \u2014 {1}\uAC1C\uC758 \uC560\uD50C\uB9AC\uCF00\uC774\uC158\uC774 \uB85C\uB4DC\uB418\uC5C8\uC2B5\uB2C8\uB2E4.",
        L"{1}\uC5D0 \uB300\uD55C \uC778\uD130\uB137 \uC561\uC138\uC2A4 \uCC28\uB2E8 \uC911...",
        L"{1}\uC5D0 \uB300\uD55C \uC778\uD130\uB137 \uC561\uC138\uC2A4 \uD5C8\uC6A9 \uC911...",
        L"\u2713  \uC778\uD130\uB137 \uC561\uC138\uC2A4\uAC00 \uC131\uACF5\uC801\uC73C\uB85C \uCC28\uB2E8\uB418\uC5C8\uC2B5\uB2C8\uB2E4.",
        L"\u2713  \uC778\uD130\uB137 \uC561\uC138\uC2A4\uAC00 \uC131\uACF5\uC801\uC73C\uB85C \uD5C8\uC6A9\uB418\uC5C8\uC2B5\uB2C8\uB2E4.",
        L"\u2717  \uCC28\uB2E8\uC5D0 \uC2E4\uD328\uD588\uC2B5\uB2C8\uB2E4. \uAD00\uB9AC\uC790 \uAD8C\uD55C\uC73C\uB85C \uC2E4\uD589\uD558\uC138\uC694.",
        L"\u2717  \uD5C8\uC6A9\uC5D0 \uC2E4\uD328\uD588\uC2B5\uB2C8\uB2E4. \uAD00\uB9AC\uC790 \uAD8C\uD55C\uC73C\uB85C \uC2E4\uD589\uD558\uC138\uC694.",
        L"\uBA3C\uC800 \uC560\uD50C\uB9AC\uCF00\uC774\uC158\uC744 \uC120\uD0DD\uD558\uC138\uC694.",
        L"\uC124\uCE58 \uACBD\uB85C\uB97C \uCC3E\uC744 \uC218 \uC5C6\uC2B5\uB2C8\uB2E4.",
        L"\uC2E4\uD589 \uD30C\uC77C\uC744 \uCC3E\uC744 \uC218 \uC5C6\uC2B5\uB2C8\uB2E4.",
        L"\uC778\uD130\uB137 \uC561\uC138\uC2A4\uAC00 \uC131\uACF5\uC801\uC73C\uB85C \uCC28\uB2E8\uB418\uC5C8\uC2B5\uB2C8\uB2E4.\n\n\uBC29\uD654\uBCBD \uADDC\uCE59\uC774 \uC0DD\uC131\uB418\uC5C8\uC2B5\uB2C8\uB2E4.",
        L"\uC778\uD130\uB137 \uC561\uC138\uC2A4\uAC00 \uC131\uACF5\uC801\uC73C\uB85C \uD5C8\uC6A9\uB418\uC5C8\uC2B5\uB2C8\uB2E4.\n\n\uBC29\uD654\uBCBD \uADDC\uCE59\uC774 \uC81C\uAC70\uB418\uC5C8\uC2B5\uB2C8\uB2E4.",
        L"\uC778\uD130\uB137 \uC561\uC138\uC2A4 \uCC28\uB2E8\uC5D0 \uC2E4\uD328\uD588\uC2B5\uB2C8\uB2E4.\n\n\uAD00\uB9AC\uC790 \uAD8C\uD55C\uC73C\uB85C \uC2E4\uD589\uD558\uC138\uC694.",
        L"\uC778\uD130\uB137 \uC561\uC138\uC2A4 \uD5C8\uC6A9\uC5D0 \uC2E4\uD328\uD588\uC2B5\uB2C8\uB2E4.\n\n\uAD00\uB9AC\uC790 \uAD8C\uD55C\uC73C\uB85C \uC2E4\uD589\uD558\uC138\uC694.",
        L"\uC815\uBCF4",
        L"\uC624\uB958",
        L"\uC131\uACF5",
        L"\uD604\uC7AC {1}/{2}\uAC1C \uC560\uD50C\uB9AC\uCF00\uC774\uC158 \uCC28\uB2E8\uC911",
        L"\uBAA8\uB450 \uD574\uC81C",
        L"\uACB0\uACFC \uC5C6\uC74C"
    },

    // ────────── Arabic ────────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \u0627\u0644\u062A\u062D\u0643\u0645 \u0641\u064A \u0627\u0644\u0648\u0635\u0648\u0644 \u0644\u0644\u0625\u0646\u062A\u0631\u0646\u062A",
        L"\U0001F504 \u062A\u062D\u062F\u064A\u062B",
        L"\u0627\u0644\u062A\u0637\u0628\u064A\u0642\u0627\u062A \u0627\u0644\u0645\u062B\u0628\u062A\u0629 ({1} \u0645\u0646 {2})",
        L"\u062C\u0627\u0631\u064D \u0627\u0644\u0639\u0645\u0644...",
        L"\u0627\u0633\u0645 \u0627\u0644\u062A\u0637\u0628\u064A\u0642",
        L"\u0627\u0644\u062D\u0627\u0644\u0629",
        L"\U0001F6AB \u0645\u062D\u0638\u0648\u0631",
        L"\u2705 \u0645\u0633\u0645\u0648\u062D",
        L"\U0001F6AB \u062D\u0638\u0631 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A",
        L"\u2705 \u0627\u0644\u0633\u0645\u0627\u062D \u0628\u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A",
        L"\u062E\u0631\u0648\u062C",
        L"\u062C\u0627\u0631\u064D \u0641\u062D\u0635 \u0627\u0644\u062A\u0637\u0628\u064A\u0642\u0627\u062A...",
        L"\u062C\u0627\u0631\u064D \u0641\u062D\u0635 \u0627\u0644\u062A\u0637\u0628\u064A\u0642\u0627\u062A \u0648\u0642\u0648\u0627\u0639\u062F \u062C\u062F\u0627\u0631 \u0627\u0644\u0646\u0627\u0631...",
        L"\u062C\u0627\u0647\u0632 \u2014 \u062A\u0645 \u062A\u062D\u0645\u064A\u0644 {1} \u062A\u0637\u0628\u064A\u0642\u064B\u0627.",
        L"\u062C\u0627\u0631\u064D \u062D\u0638\u0631 \u0627\u0644\u0648\u0635\u0648\u0644 \u0644\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0644\u0640 {1}...",
        L"\u062C\u0627\u0631\u064D \u0627\u0644\u0633\u0645\u0627\u062D \u0628\u0627\u0644\u0648\u0635\u0648\u0644 \u0644\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0644\u0640 {1}...",
        L"\u2713  \u062A\u0645 \u062D\u0638\u0631 \u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0628\u0646\u062C\u0627\u062D.",
        L"\u2713  \u062A\u0645 \u0627\u0644\u0633\u0645\u0627\u062D \u0628\u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0628\u0646\u062C\u0627\u062D.",
        L"\u2717  \u0641\u0634\u0644 \u0627\u0644\u062D\u0638\u0631. \u0642\u0645 \u0628\u0627\u0644\u062A\u0634\u063A\u064A\u0644 \u0643\u0645\u0633\u0624\u0648\u0644.",
        L"\u2717  \u0641\u0634\u0644 \u0627\u0644\u0633\u0645\u0627\u062D. \u0642\u0645 \u0628\u0627\u0644\u062A\u0634\u063A\u064A\u0644 \u0643\u0645\u0633\u0624\u0648\u0644.",
        L"\u0627\u0644\u0631\u062C\u0627\u0621 \u062A\u062D\u062F\u064A\u062F \u062A\u0637\u0628\u064A\u0642 \u0623\u0648\u0644\u0627\u064B.",
        L"\u0644\u0645 \u064A\u062A\u0645 \u0627\u0644\u0639\u062B\u0648\u0631 \u0639\u0644\u0649 \u0645\u0633\u0627\u0631 \u0627\u0644\u062A\u062B\u0628\u064A\u062A.",
        L"\u0644\u0645 \u064A\u062A\u0645 \u0627\u0644\u0639\u062B\u0648\u0631 \u0639\u0644\u0649 \u0645\u0644\u0641\u0627\u062A \u062A\u0646\u0641\u064A\u0630\u064A\u0629.",
        L"\u062A\u0645 \u062D\u0638\u0631 \u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0628\u0646\u062C\u0627\u062D.\n\n\u062A\u0645 \u0625\u0646\u0634\u0627\u0621 \u0642\u0627\u0639\u062F\u0629 \u062C\u062F\u0627\u0631 \u0627\u0644\u0646\u0627\u0631.",
        L"\u062A\u0645 \u0627\u0644\u0633\u0645\u0627\u062D \u0628\u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A \u0628\u0646\u062C\u0627\u062D.\n\n\u062A\u0645\u062A \u0625\u0632\u0627\u0644\u0629 \u0642\u0627\u0639\u062F\u0629 \u062C\u062F\u0627\u0631 \u0627\u0644\u0646\u0627\u0631.",
        L"\u0641\u0634\u0644 \u062D\u0638\u0631 \u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A.\n\n\u0642\u0645 \u0628\u0627\u0644\u062A\u0634\u063A\u064A\u0644 \u0643\u0645\u0633\u0624\u0648\u0644.",
        L"\u0641\u0634\u0644 \u0627\u0644\u0633\u0645\u0627\u062D \u0628\u0627\u0644\u0648\u0635\u0648\u0644 \u0625\u0644\u0649 \u0627\u0644\u0625\u0646\u062A\u0631\u0646\u062A.\n\n\u0642\u0645 \u0628\u0627\u0644\u062A\u0634\u063A\u064A\u0644 \u0643\u0645\u0633\u0624\u0648\u0644.",
        L"\u0645\u0639\u0644\u0648\u0645\u0627\u062A",
        L"\u062E\u0637\u0623",
        L"\u0646\u062C\u0627\u062D",
        L"\u062D\u0627\u0644\u064A\u064B\u0627 \u0647\u0646\u0627\u0643 {1} \u0645\u0646 {2} \u062A\u0637\u0628\u064A\u0642\u0627\u062A \u0645\u062D\u0638\u0648\u0631\u0629",
        L"\u0625\u0644\u063A\u0627\u0621 \u062D\u0638\u0631 \u0627\u0644\u0643\u0644",
        L"\u0644\u0627 \u0646\u062A\u0627\u0626\u062C"
    },

    // ────────── Turkish ───────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 \u0130nternet Eri\u015Fim Denetleyicisi",
        L"\U0001F504 Yenile",
        L"Y\u00FCkl\u00FC Uygulamalar ({1}/{2})",
        L"\u00C7al\u0131\u015F\u0131yor...",
        L"Uygulama Ad\u0131",
        L"Durum",
        L"\U0001F6AB Engellendi",
        L"\u2705 \u0130zin Verildi",
        L"\U0001F6AB \u0130nterneti Engelle",
        L"\u2705 \u0130nternete \u0130zin Ver",
        L"\u00C7\u0131k\u0131\u015F",
        L"Y\u00FCkl\u00FC uygulamalar taran\u0131yor...",
        L"Uygulamalar ve g\u00FCvenlik duvar\u0131 kurallar\u0131 taran\u0131yor...",
        L"Haz\u0131r \u2014 {1} uygulama y\u00FCklendi.",
        L"{1} i\u00E7in \u0130nternet eri\u015Fimi engelleniyor...",
        L"{1} i\u00E7in \u0130nternet eri\u015Fimine izin veriliyor...",
        L"\u2713  \u0130nternet eri\u015Fimi ba\u015Far\u0131yla engellendi.",
        L"\u2713  \u0130nternet eri\u015Fimine ba\u015Far\u0131yla izin verildi.",
        L"\u2717  Engelleme ba\u015Far\u0131s\u0131z. Y\u00F6netici olarak \u00E7al\u0131\u015Ft\u0131r\u0131n.",
        L"\u2717  \u0130zin verme ba\u015Far\u0131s\u0131z. Y\u00F6netici olarak \u00E7al\u0131\u015Ft\u0131r\u0131n.",
        L"L\u00FCtfen \u00F6nce bir uygulama se\u00E7in.",
        L"Kurulum yolu bulunamad\u0131.",
        L"\u00C7al\u0131\u015Ft\u0131r\u0131labilir dosya bulunamad\u0131.",
        L"\u0130nternet eri\u015Fimi ba\u015Far\u0131yla engellendi.\n\nG\u00FCvenlik duvar\u0131 kural\u0131 olu\u015Fturuldu.",
        L"\u0130nternet eri\u015Fimine ba\u015Far\u0131yla izin verildi.\n\nG\u00FCvenlik duvar\u0131 kural\u0131 kald\u0131r\u0131ld\u0131.",
        L"\u0130nternet eri\u015Fimi engellenemedi.\n\nY\u00F6netici olarak \u00E7al\u0131\u015Ft\u0131r\u0131n.",
        L"\u0130nternet eri\u015Fimine izin verilemedi.\n\nY\u00F6netici olarak \u00E7al\u0131\u015Ft\u0131r\u0131n.",
        L"Bilgi",
        L"Hata",
        L"Ba\u015Far\u0131",
        L"\u015Eu anda {1}/{2} uygulama engellendi",
        L"T\u00FCm\u00FCn\u00FC Engelini Kald\u0131r",
        L"Sonu\u00E7 yok"
    },

    // ────────── Polish ────────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Kontrola Dost\u0119pu do Internetu",
        L"\U0001F504 Od\u015Bwie\u017C",
        L"Zainstalowane aplikacje ({1} z {2})",
        L"Pracuj\u0119...",
        L"Nazwa aplikacji",
        L"Status",
        L"\U0001F6AB Zablokowana",
        L"\u2705 Dozwolona",
        L"\U0001F6AB Blokuj Internet",
        L"\u2705 Zezw\u00F3l na Internet",
        L"Wyj\u015Bcie",
        L"Skanowanie zainstalowanych aplikacji...",
        L"Skanowanie aplikacji i regu\u0142 zapory...",
        L"Gotowe \u2014 za\u0142adowano {1} aplikacji.",
        L"Blokowanie dost\u0119pu do Internetu dla {1}...",
        L"Zezwalanie na dost\u0119p do Internetu dla {1}...",
        L"\u2713  Dost\u0119p do Internetu zablokowany pomy\u015Blnie.",
        L"\u2713  Dost\u0119p do Internetu dozwolony pomy\u015Blnie.",
        L"\u2717  Blokowanie nie powiod\u0142o si\u0119. Uruchom jako Administrator.",
        L"\u2717  Zezwolenie nie powiod\u0142o si\u0119. Uruchom jako Administrator.",
        L"Najpierw wybierz aplikacj\u0119.",
        L"Nie znaleziono \u015Bcie\u017Cki instalacji.",
        L"Nie znaleziono plik\u00F3w wykonywalnych.",
        L"Dost\u0119p do Internetu zablokowany pomy\u015Blnie.\n\nRegu\u0142a zapory zosta\u0142a utworzona.",
        L"Dost\u0119p do Internetu dozwolony pomy\u015Blnie.\n\nRegu\u0142a zapory zosta\u0142a usuni\u0119ta.",
        L"Blokowanie dost\u0119pu do Internetu nie powiod\u0142o si\u0119.\n\nUruchom jako Administrator.",
        L"Zezwolenie na dost\u0119p do Internetu nie powiod\u0142o si\u0119.\n\nUruchom jako Administrator.",
        L"Informacja",
        L"B\u0142\u0105d",
        L"Sukces",
        L"Obecnie {1} z {2} aplikacji jest zablokowanych",
        L"Odblokuj Wszystkie",
        L"Brak wynik\u00F3w"
    },

    // ────────── Swedish ───────────────────────────────────────────────────
    {
        L"APP-NETWORK_MANAGER \u2014 Internet\u00E5tkomstkontroll",
        L"\U0001F504 Uppdatera",
        L"Installerade appar ({1} av {2})",
        L"Arbetar...",
        L"Applikationsnamn",
        L"Status",
        L"\U0001F6AB Blockerad",
        L"\u2705 Till\u00E5ten",
        L"\U0001F6AB Blockera Internet",
        L"\u2705 Till\u00E5t Internet",
        L"Avsluta",
        L"S\u00F6ker efter installerade appar...",
        L"S\u00F6ker efter appar och brandv\u00E4ggsregler...",
        L"Redo \u2014 {1} appar inl\u00E4stade.",
        L"Blockerar internet\u00E5tkomst f\u00F6r {1}...",
        L"Till\u00E5ter internet\u00E5tkomst f\u00F6r {1}...",
        L"\u2713  Internet\u00E5tkomst blockerad.",
        L"\u2713  Internet\u00E5tkomst till\u00E5ten.",
        L"\u2717  Blockering misslyckades. K\u00F6r som Administrat\u00F6r.",
        L"\u2717  Till\u00E5t misslyckades. K\u00F6r som Administrat\u00F6r.",
        L"V\u00E4lj en applikation f\u00F6rst.",
        L"Installationss\u00F6kv\u00E4g hittades inte.",
        L"Inga k\u00F6rbara filer hittades.",
        L"Internet\u00E5tkomst blockerad.\n\nBrandv\u00E4ggsregeln har skapats.",
        L"Internet\u00E5tkomst till\u00E5ten.\n\nBrandv\u00E4ggsregeln har tagits bort.",
        L"Blockering av internet\u00E5tkomst misslyckades.\n\nK\u00F6r som Administrat\u00F6r.",
        L"Till\u00E5telse av internet\u00E5tkomst misslyckades.\n\nK\u00F6r som Administrat\u00F6r.",
        L"Information",
        L"Fel",
        L"Lyckades",
        L"F\u00F6r n\u00E4rvarande {1} av {2} appar blockerade",
        L"L\u00E5s Upp Alla",
        L"Inga resultat"
    }
};

// ── Public accessors ─────────────────────────────────────────────────────

const std::wstring& LanguageManager::getString(StringId id) const {
    return s_translations[static_cast<int>(m_lang)][static_cast<int>(id)];
}

std::wstring LanguageManager::getStringF(StringId id, const std::wstring& arg1) const {
    return ReplaceArgs(s_translations[static_cast<int>(m_lang)][static_cast<int>(id)], arg1);
}

std::wstring LanguageManager::getStringF(StringId id, const std::wstring& arg1,
                                         const std::wstring& arg2) const {
    return ReplaceArgs(s_translations[static_cast<int>(m_lang)][static_cast<int>(id)], arg1, arg2);
}
