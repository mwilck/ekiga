;;  Default (English) language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;  Author: ...
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
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_LICENSE_BUTTON			"Next >"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) is released under the GNU General Public License (GPL). The license is provided here for information purposes only. $_CLICK"

; Components Page
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_SECTION_TITLE			"Ekiga videophone (required)"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_SHORTCUTS_SECTION_TITLE		"Shortcuts"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menu"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_SECTION_DESCRIPTION			"Core Ekiga files and dlls"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_STARTUP_SECTION_DESCRIPTION	"Will launch Ekiga when Windows starts"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Shortcuts for starting Ekiga"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_DESKTOP_SHORTCUT_DESC		"Create a shortcut to Ekiga on the Desktop"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_STARTMENU_SHORTCUT_DESC		"Create a Start Menu entry for Ekiga"

; Ekiga Section Prompts and Texts
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_UNINSTALL_DESC			"Ekiga (remove only)"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_RUN_AT_STARTUP			"Run Ekiga at Windows startup"
!insertmacro EKIGA_MACRO_DEFAULT_STRING EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Unable to uninstall the currently installed version of Ekiga. The new version will be installed without removing the currently installed version."

; Uninstall Section Prompts
!insertmacro EKIGA_MACRO_DEFAULT_STRING un.EKIGA_UNINSTALL_ERROR_1		"The uninstaller could not find registry entries for Ekiga.$\rIt is likely that another user installed this application."
!insertmacro EKIGA_MACRO_DEFAULT_STRING un.EKIGA_UNINSTALL_ERROR_2		"You do not have permission to uninstall this application."
