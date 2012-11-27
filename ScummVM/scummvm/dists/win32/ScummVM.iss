[Setup]
AppCopyright=2012
AppName=ScummVM
AppVerName=ScummVM Git
AppPublisher=The ScummVM Team
AppPublisherURL=http://www.scummvm.org/
AppSupportURL=http://www.scummvm.org/
AppUpdatesURL=http://www.scummvm.org/
DefaultDirName={pf}\ScummVM
DefaultGroupName=ScummVM
AllowNoIcons=true
AlwaysUsePersonalGroup=false
EnableDirDoesntExistWarning=false
Compression=lzma
OutputDir=C:\ScummVM
OutputBaseFilename=scummvm-win32
DisableStartupPrompt=true
AppendDefaultDirName=false
SolidCompression=true
DirExistsWarning=no
SetupIconFile=graphics\scummvm-install.ico
WizardImageFile=graphics\left.bmp
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: da; MessagesFile: compiler:Languages\Danish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: nb; MessagesFile: compiler:Languages\Norwegian.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: se; MessagesFile: compiler:Languages\Swedish.isl

[Icons]
Name: {group}\{cm:UninstallProgram, ScummVM}; Filename: {uninstallexe}
Name: {group}\ScummVM; Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\ScummVM (noconsole); Filename: {app}\scummvm.exe; Parameters: "--no-console"; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\Authors; Filename: {app}\AUTHORS.txt; WorkingDir: {app}; Comment: AUTHORS; Flags: createonlyiffileexists
Name: {group}\Copying; Filename: {app}\COPYING.txt; WorkingDir: {app}; Comment: COPYING; Flags: createonlyiffileexists
Name: {group}\Copying.BSD; Filename: {app}\COPYING.BSD.txt; WorkingDir: {app}; Comment: COPYING.BSD; Flags: createonlyiffileexists
Name: {group}\Copying.FREEFONT; Filename: {app}\COPYING.FREEFONT.txt; WorkingDir: {app}; Comment: COPYING.FREEFONT; Flags: createonlyiffileexists
Name: {group}\Copying.LGPL; Filename: {app}\COPYING.LGPL.txt; WorkingDir: {app}; Comment: COPYING.LGPL; Flags: createonlyiffileexists
Name: {group}\Copyright; Filename: {app}\COPYRIGHT.txt; WorkingDir: {app}; Comment: COPYRIGHT; Flags: createonlyiffileexists
;NEWS
Name: {group}\News; Filename: {app}\NEWS.txt; WorkingDir: {app}; Comment: NEWS; Flags: createonlyiffileexists; Languages: not de
Name: {group}\Neues; Filename: {app}\Neues.txt; WorkingDir: {app}; Comment: Neues; Flags: createonlyiffileexists; Languages: de
;QUICKSTART
Name: {group}\QuickStart; Filename: {app}\QUICKSTART.txt; WorkingDir: {app}; Comment: QUICKSTART; Flags: createonlyiffileexists; Languages: not (de or es or fr or it or nb or se)
Name: {group}\Schnellstart; Filename: {app}\Schnellstart.txt; WorkingDir: {app}; Comment: Schnellstart; Flags: createonlyiffileexists; Languages: de
Name: {group}\InicioRapido; Filename: {app}\InicioRapido.txt; WorkingDir: {app}; Comment: InicioRapido; Flags: createonlyiffileexists; Languages: es
Name: {group}\DemarrageRapide; Filename: {app}\DemarrageRapide.txt; WorkingDir: {app}; Comment: DemarrageRapide; Flags: createonlyiffileexists; Languages: fr
Name: {group}\GuidaRapida; Filename: {app}\GuidaRapida.txt; WorkingDir: {app}; Comment: GuidaRapida; Flags: createonlyiffileexists; Languages: it
Name: {group}\HurtigStart; Filename: {app}\HurtigStart.txt; WorkingDir: {app}; Comment: HurtigStart; Flags: createonlyiffileexists; Languages: nb
Name: {group}\Snabbstart; Filename: {app}\Snabbstart.txt; WorkingDir: {app}; Comment: Snabbstart; Flags: createonlyiffileexists; Languages: se
;README
Name: {group}\Readme; Filename: {app}\README.txt; WorkingDir: {app}; Comment: README; Flags: createonlyiffileexists; Languages: not (cz or de or se)
Name: {group}\PrectiMe; Filename: {app}\PrectiMe.txt; WorkingDir: {app}; Comment: PrectiMe; Flags: createonlyiffileexists; Languages: cz
Name: {group}\Liesmich; Filename: {app}\Liesmich.txt; WorkingDir: {app}; Comment: Liesmich; Flags: createonlyiffileexists; Languages: de
Name: {group}\LasMig; Filename: {app}\LasMig.txt; WorkingDir: {app}; Comment: LasMig; Flags: createonlyiffileexists; Languages: se

Name: {group}\Saved Games\Migrate Saved Games; Filename: {app}\migration.bat; WorkingDir: {app}; Comment: Migrate Saved Games; IconIndex: 0; MinVersion: 0, 1
Name: {group}\Saved Games\Saved Games (old default); Filename: {app}; WorkingDir: {app}; Comment: Saved Games (old default); IconIndex: 0; MinVersion: 0, 1
Name: {group}\Saved Games\Saved Games (new default); Filename: {userappdata}\ScummVM\Saved Games; WorkingDir: {userappdata}\ScummVM\Saved Games; Comment: Saved Games (new default); IconIndex: 0; MinVersion: 0, 1

[Run]
Filename: {app}\ScummVM.exe; Flags: nowait skipifdoesntexist postinstall skipifsilent
;Creates a symbolic link for standard save games area, under Windows Vista/7
Filename: {cmd}; Parameters: "/c ""mklink /d ""%userprofile%\Saved Games\ScummVM"" ""%appdata%\ScummVM\Saved games"" "" "; MinVersion: 0, 6.1

[UninstallDelete]
Type: files; Name: {app}\ISTool.url

[Dirs]
Name: "{userappdata}\ScummVM"; MinVersion: 0, 1
Name: "{userappdata}\ScummVM\Saved Games"; MinVersion: 0, 1

[Files]
Source: AUTHORS.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.BSD.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.FREEFONT.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.LGPL.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYRIGHT.txt; DestDir: {app}; Flags: ignoreversion
;NEWS
Source: NEWS.txt; DestDir: {app}; Flags: ignoreversion; Languages: not de
Source: doc/de/Neues.txt; DestDir: {app}; Flags: ignoreversion; Languages: de
;QUICKSTART
Source: doc/QUICKSTART.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: not (de or es or fr or it or nb or se)
Source: doc/de/Schnellstart.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: de
Source: doc/es/InicioRapido.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: es
Source: doc/fr/DemarrageRapide.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: fr
Source: doc/it/GuidaRapida.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: it
Source: doc/no-nb/HurtigStart.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: nb
Source: doc/se/Snabbstart.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: se
;README
Source: README.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: not (cz or de or se)
Source: doc/cz/PrectiMe.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: cz
Source: doc/de/Liesmich.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: de
Source: doc/se/LasMig.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: se
Source: README-SDL.txt; DestDir: {app}; Flags: ignoreversion
Source: scummvm.exe; DestDir: {app}; Flags: ignoreversion
Source: SDL.dll; DestDir: {app}
;Mirgration script for saved games in Windows NT4 onwards
Source: migration.bat; DestDir: {app}; Flags: ignoreversion; MinVersion: 0, 1
Source: migration.txt; DestDir: {app}; Flags: ignoreversion; MinVersion: 0, 1


