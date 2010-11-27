;;  Russian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1251
;;  Author: Alexey Loukianov a.k.a. LeXa2
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
!define INSTALLER_IS_RUNNING			"Программа установки уже запущена."
!define EKIGA_IS_RUNNING				"Видеотелефон Ekiga уже запущен. Попробуйте выйти из него и запустить программу установки ещё раз."
!define GTK_INSTALLER_NEEDED			"На вайшей системе отсутствуют либо требуют обновления библиотеки GTK+ runtime.$\rПожалуйста, установите GTK+ runtime версии v${GTK_VERSION} или новее."

; License Page
!define EKIGA_LICENSE_BUTTON			"Далее >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) выпускается по схеме лицензирования GNU General Public License (GPL). Представленный тут текст лицензии приводится только для справки. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Видеотелефон Ekiga (обязательно)"
!define GTK_SECTION_TITLE			"Библиотеки GTK+ Runtime (обязательно)"
!define GTK_THEMES_SECTION_TITLE			"Темы GTK+"
!define GTK_NOTHEME_SECTION_TITLE		"Без тем"
!define GTK_WIMP_SECTION_TITLE			"Тема Wimp"
!define GTK_BLUECURVE_SECTION_TITLE		"Тема Bluecurve"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Тема Light House Blue"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Ярлыки"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Рабочий стол"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Меню Пуск"
!define EKIGA_SECTION_DESCRIPTION			"Основные библиотеки и файлы Ekiga"
!define GTK_SECTION_DESCRIPTION			"Кросс-платформенные библиотеки пользовательского интерфейса, используемые Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"Темы GTK+ применяются для изменения внешнего вида приложений, использующих библиотеки GTK+."
!define GTK_NO_THEME_DESC			"Не устанавливать темы GTK+"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows impersonator) это тема GTK, которая старается повторить внешний вид обычных приложений Windows."
!define GTK_BLUECURVE_THEME_DESC			"Тема Bluecurve."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"Тема Lighthouseblue."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Автозапуск Ekiga при старте Windows"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Ярлыки для запуска Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Создать ярлык для Ekiga на Рабочем столе"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Создать для Ekiga пункт в Меню Пуск"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"Обнаружена устаревшая версия библиотек GTK+. Произвести обновление?$\rВнимание: обновление может быть необходимо для запуска Ekiga."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Посетить веб-сайт Ekiga для Windows"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (только удаление)"
!define EKIGA_RUN_AT_STARTUP			"Запускать Ekiga при старте Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Ошибка удаления уже установленной версии Ekiga. Установка новой версии будет продолжена без удаления существующей версии."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Ошибка установки библиотек GTK+. Игнорировать и продолжить установку?"
!define GTK_BAD_INSTALL_PATH			"Не удаётся получить доступ или создать папку по указанному вами пути."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"У вас отсутствуют необходимые разрешения для установки тем GTK+."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Программе удаления не удаётся найти записи реестра для Ekiga.$\rВероятнее всего видеотелефон Ekiga был установлен из под учётной записи другого пользователя."
!define un.EKIGA_UNINSTALL_ERROR_2		"У вас отсутствуют необходимые резрешения для удаления этого приложения."
