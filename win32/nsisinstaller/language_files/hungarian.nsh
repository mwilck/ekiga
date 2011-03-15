;;  Hungarian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252

; Startup Checks
!define INSTALLER_IS_RUNNING			"A telepítõprogram már fut."
!define EKIGA_IS_RUNNING				"A Ekiga jelenleg fut. Lépjen ki a Ekigaból, és próbálja újra."
!define GTK_INSTALLER_NEEDED			"A GTK+ futtatási környezet vagy hiányzik, vagy újabb verziójára van szükség.$\rTelepítse a GTK+ futtatási környezet v${GTK_VERSION} vagy újabb változatát"

; License Page
!define EKIGA_LICENSE_BUTTON			"Következõ >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"Az $(^Name) a GNU General Public License (GPL) alatt kerül forgalomba hozatalra. Itt a licenc kizárólag információs célokat szolgál. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videótelefon (szükséges)"
!define GTK_SECTION_TITLE			"GTK+ futtatási környezet (szükséges)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ témák"
!define GTK_NOTHEME_SECTION_TITLE		"Nincs téma"
!define GTK_WIMP_SECTION_TITLE			"Wimp téma"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve téma"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue téma"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Parancsikonok"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Asztal"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menü"
!define EKIGA_SECTION_DESCRIPTION			"Alapvetõ Ekiga fájlok és dll fájlok"
!define GTK_SECTION_DESCRIPTION			"Az Ekiga által használt többplatformos GUI eszközkészlet"
!define GTK_THEMES_SECTION_DESCRIPTION		"A GTK+ témák megváltoztatják a GTK+ alkalmazások megjelenését."
!define GTK_NO_THEME_DESC			"Ne telepítsen GTK+ témát"
!define GTK_WIMP_THEME_DESC			"A GTK-Wimp (Windows megszemélyesítõ) olyan  GTK téma, amely jól illeszkedik a Windows asztali környezetébe."
!define GTK_BLUECURVE_THEME_DESC			"The Bluecurve téma."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"A Lighthouseblue téma."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Az Ekiga indítása a Windows indításakor"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Parancsikonok a Ekiga indításához"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Parancsikon létrehozása az asztalon az Ekiga számára"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Start Menü bejegyzés létrehozása az Ekiga számára"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"A rendszer egy régebbi GTK+ futtatási környezetet talált. Kívánja frissíteni?$\rMegjegyzés: Amennyiben nem végzi el a frissítést, elõfordulhat hogy az Ekiga  nem fog mûködni."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Látogassa meg a windowsos Ekiga weboldalát"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (csak eltávolítás)"
!define EKIGA_RUN_AT_STARTUP			"A Ekiga futtatása a Windows indításakor"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"A rendszer nem képes az Ekiga jelenleg telepített verziójának eltávolítására. Az új verzió a jelenleg telepített változat eltávolítása nélkül kerül telepítésre."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Hiba a GTK+ futtatási környezet telepítése közben."
!define GTK_BAD_INSTALL_PATH			"A megadott elérési út nem érhetõ el, vagy nem hozható létre."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Nem jogosult a GTK+ téma telepítéséhez."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Az eltávolító nem talált Ekiga beállításjegyzék-bejegyzéseket.$\rAz alkalmazást valószínûleg másik felhasználó telepítette."
!define un.EKIGA_UNINSTALL_ERROR_2		"Nem jogosult az alkalmazás eltávolítására."
