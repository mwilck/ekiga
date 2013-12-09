;;  Dutch language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;  Author: Pieter van der Wolk <pwolk@dds.nl>

; License Page
!define EKIGA_LICENSE_BUTTON			"Next >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) is released under the GNU General Public License (GPL). The license is provided here for information purposes only. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videophone (required)"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Snelkoppelingen"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Bureaublad"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menu"
!define EKIGA_SECTION_DESCRIPTION			"Ekiga kernbestanden en bibliotheken"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Start Ekiga als Windows opstart"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Snelkoppelingen om Ekiga te starten"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Maak een Snelkoppeling naar Ekiga op het bureaublad"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Maak een Start Menu icoon voor Ekiga"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (alleen verwijderen)"
!define EKIGA_RUN_AT_STARTUP			"Start Ekiga bij starten Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"De al geinstalleerde versie van Ekiga kan niet worden verwijderd. De nieuwe versie wordt geinstalleerd zonder de oude te verwijderen."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Het de-installatieprogramma heeft geen Ekiga gegevens in het register gevonden.$\rWaarschijnlijk installeerde een andere gebruiker Ekiga."
!define un.EKIGA_UNINSTALL_ERROR_2		"U kunt dit programma niet de-installeren."
