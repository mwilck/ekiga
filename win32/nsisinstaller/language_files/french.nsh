;;  French language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Eric Boumaour <zongo_fr@users.sourceforge.net>, 2003-2005.

; License Page
!define EKIGA_LICENSE_BUTTON			"Suivant >"
!define EKIGA_LICENSE_BOTTOM_TEXT		"$(^Name) est disponible sous licence GNU General Public License (GPL). Le texte de licence suivant est fourni uniquement à titre informatif. $_CLICK" 

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videophone (obligatoire)"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Raccourcis"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Bureau"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Menu Démarrer"
!define EKIGA_SECTION_DESCRIPTION		"Fichiers et DLLs de base de Ekiga"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Exécuter Ekiga quand Windows démarre"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Raccourcis pour lancer Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Créer un raccourci pour Ekiga sur le bureau"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Créer un raccourci pour Ekiga dans le menu Démarrer"
!define EKIGA_RUN_AT_STARTUP			"Lancer Ekiga au démarrage de Windows"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (supprimer uniquement)"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Impossible de supprimer la version courante d'Ekiga. La nouvelle version écrasera la précédente."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Le programme de désinstallation n'a pas retrouvé les entrées de Ekiga dans la base de registres.$\rL'application a peut-être été installée par un utilisateur différent."
!define un.EKIGA_UNINSTALL_ERROR_2		"Vous n'avez pas les permissions pour supprimer cette application."
