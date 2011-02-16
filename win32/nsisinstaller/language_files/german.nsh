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

; Startup Checks
!define INSTALLER_IS_RUNNING			"Die Installation läuft bereits."
!define EKIGA_IS_RUNNING				"Eine Instanz von Ekiga läuft bereits. Beenden Sie Ekiga und versuchen Sie es erneut."
!define GTK_INSTALLER_NEEDED			"Die GTK+ Laufzeitumgebung (runtime environment) fehlt oder muss aktualisiert werden.$\rBitte installieren Sie die GTK Laufzeitumgebung v${GTK_VERSION} oder höher."

; License Page
!define EKIGA_LICENSE_BUTTON			"Weiter >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) ist herausgegeben unter der GNU General Public License (GPL). Die hier dargestellte Lizenz dient nur zu Ihrer Information. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga Videotelefon (benötigt)"
!define GTK_SECTION_TITLE			"GTK+ Laufzeitumgebung (benötigt)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ Motive"
!define GTK_NOTHEME_SECTION_TITLE		"Kein Motiv"
!define GTK_WIMP_SECTION_TITLE			"Wimp Motiv"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve Motiv"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue Motiv"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Symbole"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Startmenü"
!define EKIGA_SECTION_DESCRIPTION			"Ekiga Basis Dateien und dlls"
!define GTK_SECTION_DESCRIPTION			"Ein Multi-Platform GUI Werkzeugsatz benutzt von Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"GTK+ Motive können das Aussehen von GTK+ Anwendungen verändern."
!define GTK_NO_THEME_DESC			"Kein GTK+ Motiv installieren."
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows Imitator) ist ein GTK Motiv, dass sich gut in die Windowsumgebung einpasst."
!define GTK_BLUECURVE_THEME_DESC			"Das Bluecurve Motiv."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"Das Lighthouseblue Motiv."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Ekiga beim Start von Windows starten."
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Symbole zum starten von Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Erstelle ein Symbol für Ekiga auf dem Desktop"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Ein Startmenü Eintrag für Ekiga anlegen."

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"Eine alte Version der GTK+ Laufzeitumgebung (runtime environment) wurde gefunden. Möchten Sie sie aktualisieren?$\rHinweis: Ekiga wird sonst nicht funktionieren."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Besuchen Sie die Windows Ekiga Webseite"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (nur entfernen)"
!define EKIGA_RUN_AT_STARTUP			"Starte Ekiga beim Windows Start"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Kann die bereits installierte Version von Ekiga nicht deinstallieren. Die neue Version wird installiert, ohne die bereits installierte Version zu entfernen."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Fehler beim installieren der GTK+ Laufzeitumgebung (runtime environment). Möchten Sie trotzdem fortfahren?"
!define GTK_BAD_INSTALL_PATH			"Auf den Pfad den Sie angegeben haben, kann nicht zugegriffen oder er kann nicht erstellt werden."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Sie haben nicht die Berechtigung ein GTK+ Motiv zu installieren."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Das Deinstallationsprogramm kann die Registrierungseіnträge von Ekiga nicht finden.$\rEs ist wahrscheinlich, dass ein anderer Benutzer diese Anwendung installiert hat."
!define un.EKIGA_UNINSTALL_ERROR_2		"Sie haben nicht die Berechtigung diese Anwendung zu deinstallieren."
