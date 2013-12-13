; NSIS Installer for Ekiga Win32
; Original Authors: Herman Bloggs <hermanator12002@yahoo.com>
; and Daniel Atallah <daniel_atallah@yahoo.com> (GAIM Installler)

!addPluginDir ${NSISPLUGINDIR}
; ===========================
; Global Variables
var name
var STARTUP_RUN_KEY
; ===========================
; Configuration

Name $name
SetCompressor /SOLID lzma
!if ${DEBUG}
  OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}-debug.exe"
!else
  OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}.exe"
!endif

; ===========================
; Includes
!include "MUI.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!include "Library.nsh"
!include "WordFunc.nsh"
!include "${NSISSYSTEMDIR}/System.nsh"

!insertmacro GetParameters
!insertmacro GetOptions
!insertmacro GetParent

; ===========================
; Defines

!define EKIGA_REG_KEY			"SOFTWARE\ekiga"
!define EKIGA_UNINST_EXE		"ekiga-uninst.exe"
!define EKIGA_UNINSTALL_KEY		"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Ekiga"
!define HKLM_APP_PATHS_KEY 		"SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\ekiga.exe"
!define EKIGA_REG_LANG		   	"Installer Language"
!define EKIGA_STARTUP_RUN_KEY	"SOFTWARE\Microsoft\Windows\CurrentVersion\Run"

; ===========================
; Modern UI configuration
!define MUI_ICON                "${EKIGA_DIR}/win32/ico/ekiga.ico"
!define MUI_UNICON              "${EKIGA_DIR}/win32/ico/ekiga-uninstall.ico"

!define MUI_HEADERIMAGE
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_ABORTWARNING

;Finish Page config
!define MUI_FINISHPAGE_RUN			"$INSTDIR\ekiga.exe"
!define MUI_FINISHPAGE_RUN_CHECKED

; ===========================
; Pages

!insertmacro MUI_PAGE_WELCOME

; Alter License section
!define MUI_LICENSEPAGE_BUTTON		  $(EKIGA_LICENSE_BUTTON)
!define MUI_LICENSEPAGE_TEXT_BOTTOM	  $(EKIGA_LICENSE_BOTTOM_TEXT)
!insertmacro MUI_PAGE_LICENSE         "${EKIGA_DIR}/LICENSE"

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; ===========================
; Languages

;!define MUI_LANGDLL_ALLLANGUAGES  ; show all languages during install
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"

!define EKIGA_DEFAULT_LANGFILE "${INSTALLER_DIR}/language_files/english.nsh"

!include "${INSTALLER_DIR}/langmacros.nsh"

!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "ENGLISH"		"${INSTALLER_DIR}/language_files/english.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "DUTCH"		"${INSTALLER_DIR}/language_files/dutch.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "FRENCH"		"${INSTALLER_DIR}/language_files/french.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "GERMAN"		"${INSTALLER_DIR}/language_files/german.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "HUNGARIAN"		"${INSTALLER_DIR}/language_files/hungarian.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "POLISH"		"${INSTALLER_DIR}/language_files/polish.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "ROMANIAN"		"${INSTALLER_DIR}/language_files/romanian.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "RUSSIAN"		"${INSTALLER_DIR}/language_files/russian.nsh"

; ===========================
; Section SecUninstallOldEkiga
; ===========================
Section -SecUninstallOldEkiga
        ; Check install rights..
        Call CheckUserInstallRights
        Pop $R0

        ;If ekiga is currently set to run on startup,
        ;  save the section of the Registry where the setting is before uninstalling,
        ;  so we can put it back after installing the new version
        ClearErrors
        ReadRegStr $STARTUP_RUN_KEY HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
        IfErrors +3
        StrCpy $STARTUP_RUN_KEY "HKCU"
        Goto +4
        ClearErrors
        ReadRegStr $STARTUP_RUN_KEY HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
        IfErrors +2
        StrCpy $STARTUP_RUN_KEY "HKLM"

        StrCmp $R0 "HKLM" ekiga_hklm
        StrCmp $R0 "HKCU" ekiga_hkcu done

        ekiga_hkcu:
                ReadRegStr $R1 HKCU ${EKIGA_REG_KEY} ""
                ReadRegStr $R2 HKCU ${EKIGA_REG_KEY} "Version"
                ReadRegStr $R3 HKCU "${EKIGA_UNINSTALL_KEY}" "UninstallString"
                Goto try_uninstall

        ekiga_hklm:
                ReadRegStr $R1 HKLM ${EKIGA_REG_KEY} ""
                ReadRegStr $R2 HKLM ${EKIGA_REG_KEY} "Version"
                ReadRegStr $R3 HKLM "${EKIGA_UNINSTALL_KEY}" "UninstallString"

        ; If previous version exists .. remove
        try_uninstall:
                StrCmp $R1 "" done
                ; Version key started with 0.60a3. Prior versions can't be
                ; automatically uninstalled.
                StrCmp $R2 "" uninstall_problem
                ; Check if we have uninstall string..
                IfFileExists $R3 0 uninstall_problem
                ; Have uninstall string.. go ahead and uninstall.
                ; but before, prevent removal of non-standard
                ;   installation directory of ekiga prior to April 2010
                ; so the lines until nameok1 label could be removed by 2012
                ${GetFileName} $R1 $R5
                StrCmp $R5 ekiga nameok1 0  ; unsensitive comparation
                MessageBox MB_OK "WARNING: Ekiga was installed in $R1, which is not a standard location.  Your old ekiga files will not be removed, please remove manually the directory $R1 after ensuring that you have not added to it useful files for you."
                Goto done
                nameok1:
                SetOverwrite on
                ; Need to copy uninstaller outside of the install dir
                ClearErrors
                CopyFiles /SILENT $R3 "$TEMP\${EKIGA_UNINST_EXE}"
                SetOverwrite off
                IfErrors uninstall_problem
                ; Ready to uninstall..
                ClearErrors
                ExecWait '"$TEMP\${EKIGA_UNINST_EXE}" /S _?=$R1'
                IfErrors exec_error
                Delete "$TEMP\${EKIGA_UNINST_EXE}"
                Goto done

        exec_error:
                Delete "$TEMP\${EKIGA_UNINST_EXE}"
                Goto uninstall_problem

        uninstall_problem:
                ; We can't uninstall.  Either the user must manually uninstall or we ignore and reinstall over it.
                MessageBox MB_OKCANCEL $(EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL) /SD IDOK IDOK done
                Quit

        done:
SectionEnd

; ===========================
; Section SecEkiga
; ===========================
Section $(EKIGA_SECTION_TITLE) SecEkiga
  SectionIn 1 RO

  ; find out a good installation directory, allowing the uninstaller
  ;   to safely remove the whole installation directory
  ; if INSTDIR does not end in [Ee]kiga, then add subdir Ekiga
  ${GetFileName} $INSTDIR $R0
  StrCmp $R0 ekiga nameok 0  ; unsensitive comparation
  StrCpy $INSTDIR "$INSTDIR\Ekiga"

  nameok:
  ; if exists and not empty, then add subdir Ekiga
  IfFileExists $INSTDIR 0 dirok
  ${DirState} $INSTDIR $R0
  IntCmp $R0 0 dirok
  StrCpy $INSTDIR "$INSTDIR\Ekiga"

  ; if exists, abort
  IfFileExists $INSTDIR 0 dirok
  abort "Error: tried $INSTDIR, but it already exists.  Please restart the setup and specify another installation directory"

  dirok:
  ; check install rights
  Call CheckUserInstallRights
  Pop $R0

  StrCmp $R0 "NONE" ekiga_install_files
  StrCmp $R0 "HKLM" ekiga_hklm ekiga_hkcu

  ekiga_hklm:
    WriteRegStr HKLM "${HKLM_APP_PATHS_KEY}" "" "$INSTDIR\ekiga.exe"
    WriteRegStr HKLM ${EKIGA_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKLM ${EKIGA_REG_KEY} "Version" "${EKIGA_VERSION}"
    WriteRegStr HKLM "${EKIGA_UNINSTALL_KEY}" "DisplayName" $(EKIGA_UNINSTALL_DESC)
    WriteRegStr HKLM "${EKIGA_UNINSTALL_KEY}" "UninstallString" "$INSTDIR\${EKIGA_UNINST_EXE}"
    ; Set scope of the desktop and Start Menu entries for all users
    SetShellVarContext "all"
    Goto ekiga_install_files

  ekiga_hkcu:
    WriteRegStr HKCU ${EKIGA_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKCU ${EKIGA_REG_KEY} "Version" "${EKIGA_VERSION}"
    WriteRegStr HKCU "${EKIGA_UNINSTALL_KEY}" "DisplayName" $(EKIGA_UNINSTALL_DESC)
    WriteRegStr HKCU "${EKIGA_UNINSTALL_KEY}" "UninstallString" "$INSTDIR\${EKIGA_UNINST_EXE}"

  ekiga_install_files:
    SetOutPath "$INSTDIR"
    ; Ekiga files
    SetOverwrite on
    File /r "${TARGET_DIR}/Ekiga/*.*"
    File "${EKIGA_DIR}/win32/ico/ekiga.ico"

    ; If we don't have install rights.. we're done
    StrCmp $R0 "NONE" done
    SetOverwrite off

    ; Write out installer language
    WriteRegStr HKCU "${EKIGA_REG_KEY}" "${EKIGA_REG_LANG}" "$LANGUAGE"

    ; write out uninstaller
    SetOverwrite on
    WriteUninstaller "$INSTDIR\${EKIGA_UNINST_EXE}"
    SetOverwrite off

    ; If we previously had ekiga setup to run on startup, make it do so again
    StrCmp $STARTUP_RUN_KEY "HKCU" +1 +2
    WriteRegStr HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga" "$INSTDIR\ekiga.exe"
    StrCmp $STARTUP_RUN_KEY "HKLM" +1 +2
    WriteRegStr HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga" "$INSTDIR\ekiga.exe"

    SetOutPath "$INSTDIR"
  done:
SectionEnd ; end of default Ekiga section

; ===========================
; Shortcuts
; ===========================
SubSection /e $(EKIGA_SHORTCUTS_SECTION_TITLE) SecShortcuts
  Section $(EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE) SecDesktopShortcut
    SetOutPath "$INSTDIR"
    SetShellVarContext "all"
    SetOverwrite on
    CreateShortCut "$DESKTOP\Ekiga.lnk" "$INSTDIR\ekiga.exe" "" "$INSTDIR\ekiga.ico"
    SetOverwrite off
    SetShellVarContext "current"
  SectionEnd

  Section $(EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE) SecStartMenuShortcut
    SetOutPath "$INSTDIR"
    SetShellVarContext "all"
    SetOverwrite on
    CreateDirectory "$SMPROGRAMS\Ekiga"
    CreateShortCut "$SMPROGRAMS\Ekiga\Ekiga.lnk" "$INSTDIR\ekiga.exe" ""  "$INSTDIR\ekiga.ico"
    CreateShortcut "$SMPROGRAMS\Ekiga\Uninstall Ekiga.lnk" "$INSTDIR\${EKIGA_UNINST_EXE}" "" "" "" "" "" "Uninstall Ekiga"
    SetOverwrite off
    SetShellVarContext "current"
  SectionEnd

  Section $(EKIGA_RUN_AT_STARTUP) SecStartup
     SetOutPath $INSTDIR
     CreateShortCut "$SMSTARTUP\Ekiga.lnk" "$INSTDIR\ekiga.exe" "" "" 0 SW_SHOWNORMAL
  SectionEnd
SubSectionEnd


; ===========================
; Section Uninstall
; ===========================
Section Uninstall
  Call un.CheckUserInstallRights
  Pop $R0
  StrCmp $R0 "NONE" no_rights
  StrCmp $R0 "HKCU" try_hkcu try_hklm

  try_hkcu:
    ReadRegStr $R0 HKCU ${EKIGA_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 cant_uninstall
    ; HKCU install path matches our INSTDIR.. so uninstall
    DeleteRegKey HKCU ${EKIGA_REG_KEY}
    DeleteRegKey HKCU "${EKIGA_UNINSTALL_KEY}"
    Goto cont_uninstall

  try_hklm:
    ReadRegStr $R0 HKLM ${EKIGA_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 try_hkcu
    ; HKLM install path matches our INSTDIR.. so uninstall
    DeleteRegKey HKLM ${EKIGA_REG_KEY}
    DeleteRegKey HKLM "${EKIGA_UNINSTALL_KEY}"
    DeleteRegKey HKLM "${HKLM_APP_PATHS_KEY}"
    ; Sets start menu and desktop scope to all users..
    SetShellVarContext "all"

  cont_uninstall:
    ; The WinPrefs plugin may have left this behind..
    DeleteRegValue HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
    DeleteRegValue HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
    ; Remove Language preference info
    DeleteRegKey HKCU ${EKIGA_REG_KEY} ;${MUI_LANGDLL_REGISTRY_ROOT} ${MUI_LANGDLL_REGISTRY_KEY}

    ; this is safe, since Ekiga was installed in an empty directory
    RMDir /r /REBOOTOK "$INSTDIR"

    SetShellVarContext "all"
    Delete /REBOOTOK "$SMPROGRAMS\Ekiga\*.*"
    Delete /REBOOTOK "$SMSTARTUP\Ekiga.lnk"
    RMDir "$SMPROGRAMS\Ekiga"
    Delete "$DESKTOP\Ekiga.lnk"

    SetShellVarContext "current"
    ; Shortcuts..
    RMDir /r "$SMPROGRAMS\Ekiga"
    Delete "$DESKTOP\Ekiga.lnk"

    Goto done

  cant_uninstall:
    MessageBox MB_OK $(un.EKIGA_UNINSTALL_ERROR_1) /SD IDOK
    Quit

  no_rights:
    MessageBox MB_OK $(un.EKIGA_UNINSTALL_ERROR_2) /SD IDOK
    Quit

  done:
SectionEnd ; end of uninstall section

; ===========================
; Function .onInit
; ===========================
Function .onInit
  Push $R0
  SystemLocal::Call 'kernel32::CreateMutexA(i 0, i 0, t "ekiga_installer_running") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION "Another instance of the installer is already running" /SD IDOK
  Abort
  Call RunCheck

  StrCpy $name "Ekiga"

  ${GetParameters} $R0
  ClearErrors
  ; if you wish to start with another language, execute for example:
  ; ekiga-setup.exe /L=1036
  ${GetOptions} $R0 "/L=" $R1
  IfErrors skip_lang

  StrCpy $LANGUAGE $R1
  ;!insertmacro MUI_LANGDLL_DISPLAY  ; display the language selection dialog

  skip_lang:
    ; If install path was set on the command, use it.
    StrCmp $INSTDIR "" 0 instdir_done

    ;  If ekiga is currently intalled, we should default to where it is currently installed
    ClearErrors
    ReadRegStr $INSTDIR HKCU "${EKIGA_REG_KEY}" ""
    IfErrors +2
    StrCmp $INSTDIR "" 0 instdir_done
    ClearErrors
    ReadRegStr $INSTDIR HKLM "${EKIGA_REG_KEY}" ""
    IfErrors +2
    StrCmp $INSTDIR "" 0 instdir_done

    Call CheckUserInstallRights
    Pop $R0

    StrCmp $R0 "HKLM" 0 user_dir
    StrCpy $INSTDIR "$PROGRAMFILES\Ekiga"
    Goto instdir_done

  user_dir:
    Push $SMPROGRAMS
    ${GetParent} $SMPROGRAMS $R2
    ${GetParent} $R2 $R2
    StrCpy $INSTDIR "$R2\Ekiga"

  instdir_done:
    Pop $R0
FunctionEnd

Function un.onInit
  StrCpy $name "Ekiga"
FunctionEnd


; ===========================
; Check if another instance
; of the installer is running
; ===========================
!macro RunCheckMacro UN
Function ${UN}RunCheck
  Push $R0
  Processes::FindProcess "ekiga.exe"
  IntCmp $R0 0 done
  MessageBox MB_YESNO|MB_ICONEXCLAMATION "Ekiga is running. To continue installation I need to shut it down. Shall I proceed?" /SD IDYES IDNO abort_install
  Processes::KillProcess "ekiga.exe"
  Goto done

  abort_install:
    Abort

  done:
    Pop $R0
FunctionEnd
!macroend
!insertmacro RunCheckMacro ""
!insertmacro RunCheckMacro "un."


!macro CheckUserInstallRightsMacro UN
Function ${UN}CheckUserInstallRights
  Push $0
  Push $1
  ClearErrors
  UserInfo::GetName
  IfErrors Win9x
  Pop $0
  UserInfo::GetAccountType
  Pop $1

  StrCmp $1 "Admin" 0 +3
  StrCpy $1 "HKLM"
  Goto done
  StrCmp $1 "Power" 0 +3
  StrCpy $1 "HKLM"
  Goto done
  StrCmp $1 "User" 0 +3
  StrCpy $1 "HKCU"
  Goto done
  StrCmp $1 "Guest" 0 +3
  StrCpy $1 "NONE"
  Goto done
  ; Unknown error
  StrCpy $1 "NONE"
  Goto done

  Win9x:
    StrCpy $1 "HKLM"

  done:
    Exch $1
    Exch
    Pop $0
FunctionEnd
!macroend
!insertmacro CheckUserInstallRightsMacro ""
!insertmacro CheckUserInstallRightsMacro "un."

; ===========================
; Descriptions
; ===========================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEkiga} $(EKIGA_SECTION_DESCRIPTION)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartup} $(EKIGA_STARTUP_SECTION_DESCRIPTION)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(EKIGA_SHORTCUTS_SECTION_DESCRIPTION)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopShortcut} $(EKIGA_DESKTOP_SHORTCUT_DESC)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenuShortcut} $(EKIGA_STARTMENU_SHORTCUT_DESC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
