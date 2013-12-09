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

; License Page
!define EKIGA_LICENSE_BUTTON			"Далее >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) выпускается по схеме лицензирования GNU General Public License (GPL). Представленный тут текст лицензии приводится только для справки. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Видеотелефон Ekiga (обязательно)"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Ярлыки"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Рабочий стол"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Меню Пуск"
!define EKIGA_SECTION_DESCRIPTION			"Основные библиотеки и файлы Ekiga"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Автозапуск Ekiga при старте Windows"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Ярлыки для запуска Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Создать ярлык для Ekiga на Рабочем столе"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Создать для Ekiga пункт в Меню Пуск"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (только удаление)"
!define EKIGA_RUN_AT_STARTUP			"Запускать Ekiga при старте Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Ошибка удаления уже установленной версии Ekiga. Установка новой версии будет продолжена без удаления существующей версии."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Программе удаления не удаётся найти записи реестра для Ekiga.$\rВероятнее всего видеотелефон Ekiga был установлен из под учётной записи другого пользователя."
!define un.EKIGA_UNINSTALL_ERROR_2		"У вас отсутствуют необходимые резрешения для удаления этого приложения."
