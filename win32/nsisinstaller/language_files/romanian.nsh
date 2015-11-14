;;  Romanian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Eugen Dedu <eugen.dedu@univ-fcomte.fr>, May 2010

; License Page
!define EKIGA_LICENSE_BUTTON			"Următorul >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) este distribuit sub licență GNU General Public License (GPL). Licența este dată aici numai ca informație. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Videofon Ekiga (obligatoriu)"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Scurtături"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Meniul Start"
!define EKIGA_SECTION_DESCRIPTION			"Fișiere Ekiga de bază și dll-uri"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Executa Ekiga la demararea Windows-ului"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Scurtături pentru lansarea Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Creează o scurtătură Ekiga pe Desktop"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Creează o intrare Ekiga în meniul Start"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (remove only)"
!define EKIGA_RUN_AT_STARTUP			"Lansează Ekiga la demararea Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Unable to uninstall the currently installed version of Ekiga. The new version will be installed without removing the currently installed version."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"The uninstaller could not find registry entries for Ekiga.$\rIt is likely that another user installed this application."
!define un.EKIGA_UNINSTALL_ERROR_2		"You do not have permission to uninstall this application."
