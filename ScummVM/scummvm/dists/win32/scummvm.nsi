# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

#!define _DEBUG
#!define _INCLUDE_DATA_FILES
!define _ENABLE_GAME_EXPLORER
#!define _LOG_BUILD
!define _CONVERT_TEXT

Name ScummVM

# Included files
!include MUI2.nsh

# Plugins
!ifdef _ENABLE_GAME_EXPLORER
!AddPluginDir "./plugins"
!endif

#########################################################################################
# Command line options
#########################################################################################

#!define top_srcdir   ""
#!define staging_dir  ""
#!define ARCH         ""    ;(optional, defaults to win32)

# Check parameters
!ifndef top_srcdir
	!error "Top source folder has not been passed to command line!"
!endif

!ifndef staging_dir
	!error "Staging folder has not been passed to command line (this folder should contain the executable and linked DLLs)!"
!endif

!ifndef ARCH
	!warning "ARCH has not been defined, defaulting to 'win32'"
	!define ARCH         "win32"
!else
	!if "${ARCH}" == ""
		!warning "ARCH was empty, defaulting to 'win32'"
		!undef ARCH
		!define ARCH         "win32"
	!endif
!endif

#########################################################################################
# Folders
#########################################################################################
!define engine_data  "${top_srcdir}\dists\engine-data"
!define theme_data   "${top_srcdir}\gui\themes"

#########################################################################################
# General Symbol Definitions
#########################################################################################
!define REGKEY      "Software\$(^Name)\$(^Name)"
!define VERSION     "1.6.0git"
!define COMPANY     "ScummVM Team"
!define URL         "http://scummvm.org/"
!define DESCRIPTION "ScummVM Installer. Look! A three headed monkey (TM)!"
!define COPYRIGHT   "Copyright � 2001-2012 The ScummVM Team"

#########################################################################################
# Installer configuration
#########################################################################################
OutFile          ${staging_dir}\scummvm-${VERSION}-${ARCH}.exe
InstallDir       $PROGRAMFILES\ScummVM                            ; Default installation folder
InstallDirRegKey HKCU "Software\ScummVM\ScummVM" "InstallPath"    ; Get installation folder from registry if available
                                                                  ; The application name needs to be refered directly instead of through ${REGKEY}
                                                                  ; because lang strings aren't initialized at the point InstallDirRegKey is called

CRCCheck on
XPStyle  on
#TargetMinimalOS 5.0    ; Minimal version of windows for installer: Windows 2000 or more recent
                        ; (will build unicode installer with NSIS 2.50+)

VIProductVersion 1.6.0.0
VIAddVersionKey  ProductName      $(^Name)
VIAddVersionKey  ProductVersion  "${VERSION}"
VIAddVersionKey  CompanyName     "${COMPANY}"
VIAddVersionKey  CompanyWebsite  "${URL}"
VIAddVersionKey  FileVersion     "${VERSION}"
VIAddVersionKey  FileDescription "${DESCRIPTION}"
VIAddVersionKey  LegalCopyright  "${COPYRIGHT}"

BrandingText "$(^Name) ${VERSION}"   ; Change branding text on the installer to show our name and version instead of NSIS's

# Show Details when installing/uninstalling files
ShowInstDetails   show
ShowUninstDetails show

!ifdef _DEBUG
	SetCompress off                      ; for debugging the installer, lzma takes forever
	RequestExecutionLevel user
!else
	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 64
	RequestExecutionLevel admin          ; for installation into program files folders
!endif

#########################################################################################
# MUI Symbol Definitions
#########################################################################################
!define MUI_WELCOMEFINISHPAGE_BITMAP "graphics\left.bmp"
!define MUI_ICON                     "graphics\scummvm-install.ico"
!define MUI_UNICON                   "graphics\scummvm-install.ico"

#Start menu
!define MUI_STARTMENUPAGE_REGISTRY_ROOT      HKCU
!define MUI_STARTMENUPAGE_REGISTRY_KEY       ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER      $(^Name)

# Finish page
!define MUI_FINISHPAGE_RUN        "$INSTDIR\scummvm.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

!define MUI_LICENSEPAGE_RADIOBUTTONS

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

#########################################################################################
# Installer pages
#########################################################################################
# Variables
Var StartMenuGroup

;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT      HKCU
!define MUI_LANGDLL_REGISTRY_KEY       ${REGKEY}
!define MUI_LANGDLL_REGISTRY_VALUENAME "InstallerLanguage"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ${top_srcdir}\COPYING
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!insertmacro MUI_LANGUAGE "English"    ;first language is the default language

!ifndef _DEBUG    ; Skip other languages when building debug builds
;!insertmacro MUI_LANGUAGE "Afrikaans"
;!insertmacro MUI_LANGUAGE "Albanian"
;!insertmacro MUI_LANGUAGE "Arabic"
;!insertmacro MUI_LANGUAGE "Belarusian"
;!insertmacro MUI_LANGUAGE "Bosnian"
;!insertmacro MUI_LANGUAGE "Breton"
;!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Catalan"
;!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
;!insertmacro MUI_LANGUAGE "Dutch"
;!insertmacro MUI_LANGUAGE "Esperanto"
;!insertmacro MUI_LANGUAGE "Estonian"
;!insertmacro MUI_LANGUAGE "Farsi"
;!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "French"
;!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "German"
;!insertmacro MUI_LANGUAGE "Greek"
;!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Hungarian"
;!insertmacro MUI_LANGUAGE "Icelandic"
;!insertmacro MUI_LANGUAGE "Indonesian"
;!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Italian"
;!insertmacro MUI_LANGUAGE "Japanese"
;!insertmacro MUI_LANGUAGE "Korean"
;!insertmacro MUI_LANGUAGE "Kurdish"
;!insertmacro MUI_LANGUAGE "Latvian"
;!insertmacro MUI_LANGUAGE "Lithuanian"
;!insertmacro MUI_LANGUAGE "Luxembourgish"
;!insertmacro MUI_LANGUAGE "Macedonian"
;!insertmacro MUI_LANGUAGE "Malay"
;!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Polish"
;!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
;!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
;!insertmacro MUI_LANGUAGE "Serbian"
;!insertmacro MUI_LANGUAGE "SerbianLatin"
;!insertmacro MUI_LANGUAGE "SimpChinese"
;!insertmacro MUI_LANGUAGE "Slovak"
;!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Spanish"
;!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Swedish"
;!insertmacro MUI_LANGUAGE "Thai"
;!insertmacro MUI_LANGUAGE "TradChinese"
;!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
;!insertmacro MUI_LANGUAGE "Uzbek"
!endif

;Reserve Files (will make sure the file will be stored first in the data block
;               making the installer start faster when compressing in solid mode)
!insertmacro MUI_RESERVEFILE_LANGDLL

#########################################################################################
# Installer sections
#########################################################################################
Section "ScummVM" SecMain
!ifdef _LOG_BUILD
	LogSet on
!endif
	SetOutPath $INSTDIR
	SetOverwrite on

	# Text files
	File /oname=AUTHORS.txt      "${top_srcdir}\AUTHORS"
	File /oname=COPYING.LGPL.txt "${top_srcdir}\COPYING.LGPL"
	File /oname=COPYING.txt      "${top_srcdir}\COPYING"
	File /oname=COPYRIGHT.txt    "${top_srcdir}\COPYRIGHT"
	File /oname=NEWS.txt         "${top_srcdir}\NEWS"
	File /oname=README.txt       "${top_srcdir}\README"
	
!ifdef _CONVERT_TEXT
	# Convert line endings
	Push "$INSTDIR\AUTHORS.txt"
	Call unix2dos
	Push "$INSTDIR\COPYING.LGPL.txt"
	Call unix2dos
	Push "$INSTDIR\COPYING.txt"
	Call unix2dos
	Push "$INSTDIR\COPYRIGHT.txt"
	Call unix2dos
	Push "$INSTDIR\NEWS.txt"
	Call unix2dos
	Push "$INSTDIR\README.txt"
	Call unix2dos
!endif

!ifdef _INCLUDE_DATA_FILES
	# Engine data
	File "${engine_data}\drascula.dat"
	File "${engine_data}\hugo.dat"
	File "${engine_data}\kyra.dat"
	File "${engine_data}\lure.dat"
	File "${engine_data}\queen.tbl"
	File "${engine_data}\sky.cpt"
	File "${engine_data}\teenagent.dat"
	File "${engine_data}\toon.dat"

	File "${top_srcdir}\dists\pred.dic"

	# Themes
	File "${theme_data}\scummclassic.zip"
	File "${theme_data}\scummmodern.zip"
	File "${theme_data}\translations.dat"
!endif

	# Main exe and dlls
	File "${staging_dir}\scummvm.exe"
	File "${staging_dir}\SDL.dll"

	WriteRegStr HKCU "${REGKEY}" InstallPath "$INSTDIR"    ; Store installation folder
	
	#Register with game explorer
!ifdef _ENABLE_GAME_EXPLORER
	Games::registerGame "$INSTDIR\scummvm.exe"
	pop $0
	# This is for Vista only, for 7 the tasks are defined in the gdf xml
	${If} $0 != "0"
	${AndIf} $0 != ""
	${AndIf} $0 != "$INSTDIR\scummvm.exe"
		CreateDirectory "$0\PlayTasks\0"
		CreateShortcut "$0\PlayTasks\0\Play.lnk" "$INSTDIR\scummvm.exe" "--no-console"
		CreateDirectory "$0\PlayTasks\1"
		CreateShortcut "$0\PlayTasks\1\Play (console).lnk" "$INSTDIR\scummvm.exe"
		CreateDirectory "$0\SupportTasks\0"
		CreateShortcut "$0\SupportTasks\0\Home Page.lnk" "${URL}"
	${EndIf}
!endif
SectionEnd

# Write Start menu entries and uninstaller
Section -post SecMainPost
	SetOutPath $INSTDIR
	WriteUninstaller $INSTDIR\uninstall.exe
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetShellVarContext all     ; Create shortcuts in the all-users folder
	CreateDirectory "$SMPROGRAMS\$StartMenuGroup"
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\$(^Name).lnk"              $INSTDIR\$(^Name).exe "" "$INSTDIR\$(^Name).exe" 0    ; Create shortcut with icon
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\$(^Name) (No console).lnk" $INSTDIR\$(^Name).exe "--no-console" "$INSTDIR\$(^Name).exe" 0
	CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Readme.lnk"             $INSTDIR\README.txt
	CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\uninstall.exe
	!insertmacro MUI_STARTMENU_WRITE_END
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\uninstall.exe
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\uninstall.exe
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" InstallLocation $INSTDIR
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoModify 1
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoRepair 1
SectionEnd

# Installer functions
Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY

!ifdef _DEBUG && NSIS_CONFIG_LOG
	LogSet on    ; Will write a log file to the install folder (when using the special NSIS logging build)
!endif
FunctionEnd

#########################################################################################
# Uninstaller sections
#########################################################################################
Section -un.Main SecUninstall
	Delete /REBOOTOK $INSTDIR\AUTHORS.txt
	Delete /REBOOTOK $INSTDIR\COPYING.txt
	Delete /REBOOTOK $INSTDIR\COPYING.LGPL.txt
	Delete /REBOOTOK $INSTDIR\COPYRIGHT.txt
	Delete /REBOOTOK $INSTDIR\NEWS.txt
	Delete /REBOOTOK $INSTDIR\README.txt
	Delete /REBOOTOK $INSTDIR\README-SDL.txt

!ifdef _INCLUDE_DATA_FILES
	Delete /REBOOTOK $INSTDIR\drascula.dat
	Delete /REBOOTOK $INSTDIR\hugo.dat
	Delete /REBOOTOK $INSTDIR\kyra.dat
	Delete /REBOOTOK $INSTDIR\lure.dat
	Delete /REBOOTOK $INSTDIR\queen.tbl
	Delete /REBOOTOK $INSTDIR\sky.cpt
	Delete /REBOOTOK $INSTDIR\teenagent.dat
	Delete /REBOOTOK $INSTDIR\toon.dat

	Delete /REBOOTOK $INSTDIR\pred.dic

	Delete /REBOOTOK $INSTDIR\scummclassic.zip
	Delete /REBOOTOK $INSTDIR\scummmodern.zip
	Delete /REBOOTOK $INSTDIR\translations.dat
!endif

!ifdef _ENABLE_GAME_EXPLORER
	Games::unregisterGame "$INSTDIR\scummvm.exe"
!endif

	Delete /REBOOTOK $INSTDIR\scummvm.exe
	Delete /REBOOTOK $INSTDIR\SDL.dll
SectionEnd

Section -un.post SecUninstallPost
	# Remove start menu entries
	SetShellVarContext all
	Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\$(^Name).lnk"
	Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Readme.lnk"
	Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
	RmDir  /REBOOTOK  $SMPROGRAMS\$StartMenuGroup

	Delete /REBOOTOK $INSTDIR\uninstall.exe

	DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
	DeleteRegValue HKCU "${REGKEY}" StartMenuGroup
	DeleteRegValue HKCU "${REGKEY}" InstallPath
	DeleteRegValue HKCU "${REGKEY}" InstallerLanguage
	DeleteRegKey /IfEmpty HKCU "${REGKEY}"

	RmDir $INSTDIR    ; will only remove if empty (pass /r flag for recursive behavior)
	Push $R0
	StrCpy $R0 $StartMenuGroup 1
	StrCmp $R0 ">" no_smgroup
no_smgroup:
	Pop $R0
SectionEnd

# Uninstaller functions
Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	ReadRegStr   $INSTDIR HKCU "${REGKEY}" InstallPath
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
FunctionEnd


#########################################################################################
# Helper functions
#########################################################################################

!ifdef _CONVERT_TEXT
;-------------------------------------------------------------------------------
; strips all CRs and then converts all LFs into CRLFs
; (this is roughly equivalent to "cat file | dos2unix | unix2dos")
;
; Usage:
;	Push "infile"
;	Call unix2dos
;
; Note: this function destroys $0 $1 $2
Function unix2dos
	ClearErrors

	Pop $2
	Rename $2 $2.U2D
	FileOpen $1 $2 w

	FileOpen $0 $2.U2D r

	Push $2 ; save name for deleting

	IfErrors unix2dos_done

	; $0 = file input (opened for reading)
	; $1 = file output (opened for writing)

unix2dos_loop:
	; read a byte (stored in $2)
	FileReadByte $0 $2
	IfErrors unix2dos_done ; EOL
	; skip CR
	StrCmp $2 13 unix2dos_loop
	; if LF write an extra CR
	StrCmp $2 10 unix2dos_cr unix2dos_write

unix2dos_cr:
	FileWriteByte $1 13

unix2dos_write:
	; write byte
	FileWriteByte $1 $2
	; read next byte
	Goto unix2dos_loop

unix2dos_done:
	; close files
	FileClose $0
	FileClose $1

	; delete original
	Pop $0
	Delete $0.U2D
FunctionEnd
!endif
