#define MyAppName "Engauge Digitizer"
#ifndef MyAppVersion
#define MyAppVersion "12.11.3"
#endif
#ifndef MyAppFileVersion
#define MyAppFileVersion "12.11.3.0"
#endif
#ifndef MyAppSourceDir
#define MyAppSourceDir "..\..\dist\Engauge Digitizer"
#endif
#define MyAppPublisher "Engauge Digitizer Community Build"
#define MyAppExeName "Engauge.exe"

[Setup]
AppId={{00A6792B-65ED-4894-A48B-B95D63C62CC6}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir=..\..\dist
OutputBaseFilename=Engauge-Digitizer-{#MyAppVersion}-Windows-x64-Setup
SetupIconFile=..\..\src\img\digitizer.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline
ChangesAssociations=yes
CloseApplications=yes
VersionInfoVersion={#MyAppFileVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#MyAppSourceDir}\*"; DestDir: "{app}"; Excludes: "vc_redist.x64.exe"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppSourceDir}\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
Root: HKA; Subkey: "Software\Classes\.dig"; ValueType: string; ValueName: ""; ValueData: "EngaugeDigitizer.Document"; Check: ShouldRegisterFileAssociation; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\EngaugeDigitizer.Document"; ValueType: string; ValueName: ""; ValueData: "Engauge Digitizer document"; Check: ShouldRegisterFileAssociation; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\EngaugeDigitizer.Document\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Check: ShouldRegisterFileAssociation
Root: HKA; Subkey: "Software\Classes\EngaugeDigitizer.Document\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Check: ShouldRegisterFileAssociation

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing the Microsoft Visual C++ runtime..."; Check: IsAdmin; Flags: runhidden waituntilterminated
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[Code]
function ShouldRegisterFileAssociation(): Boolean;
begin
  Result := ExpandConstant('{param:NoFileAssociation|0}') <> '1';
end;
