;;  German language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;  Author: Andreas Weisker
;;
;;  Note: To translate this file:
;;  - download this file on your computer
;;  - translate all the strings into your language
;;  - put the appropriate Windows Code Page (the one you use) above
;;  - add yourself as Author above
;;  - send us the file and remind us:
;;    - to add the entry for your file in ekiga.nsi
;;      (MUI_LANGUAGE and EKIGA_MACRO_INCLUDE_LANGFILE)
;;    - to replace everywhere in your file
;;      "!insertmacro EKIGA_MACRO_DEFAULT_STRING" with "!define"

; License Page
!define EKIGA_LICENSE_BUTTON			"Weiter >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) ist herausgegeben unter der GNU General Public License (GPL). Die hier dargestellte Lizenz dient nur zu Ihrer Information. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga Videotelefon (benötigt)"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Symbole"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Startmenü"
!define EKIGA_SECTION_DESCRIPTION			"Ekiga Basis Dateien und dlls"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Ekiga beim Start von Windows starten."
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Symbole zum starten von Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Erstelle ein Symbol für Ekiga auf dem Desktop"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Ein Startmenü Eintrag für Ekiga anlegen."

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (nur entfernen)"
!define EKIGA_RUN_AT_STARTUP			"Starte Ekiga beim Windows Start"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Kann die bereits installierte Version von Ekiga nicht deinstallieren. Die neue Version wird installiert, ohne die bereits installierte Version zu entfernen."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Das Deinstallationsprogramm kann die Registrierungseіnträge von Ekiga nicht finden.$\rEs ist wahrscheinlich, dass ein anderer Benutzer diese Anwendung installiert hat."
!define un.EKIGA_UNINSTALL_ERROR_2		"Sie haben nicht die Berechtigung diese Anwendung zu deinstallieren."
