#define MyAppName "Strategix"
#define MyAppVersion "2.0.0"
#define MyAppPublisher "Strategix"
#define MyAppExeName "strategix.exe"

[Setup]
AppId={{7B711FA7-708D-43C4-A899-9119FF202600}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}

DefaultDirName={autopf}\Strategix
DefaultGroupName=Strategix

OutputDir=..\..\release
OutputBaseFilename=Strategix-Windows-x64-Setup

Compression=lzma2
SolidCompression=yes
WizardStyle=modern

PrivilegesRequired=admin

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

UninstallDisplayIcon={app}\{#MyAppExeName}

[Tasks]
Name: "desktopicon"; \
Description: "Créer un raccourci sur le Bureau"; \
GroupDescription: "Raccourcis supplémentaires:"; \
Flags: unchecked

[Files]
Source: "..\..\dist\windows\Strategix\*"; \
DestDir: "{app}"; \
Flags: ignoreversion recursesubdirs createallsubdirs

Source: "..\..\dist\windows\vc_redist.x64.exe"; \
DestDir: "{tmp}"; \
Flags: deleteafterinstall

[Icons]
Name: "{group}\Strategix"; \
Filename: "{app}\{#MyAppExeName}"

Name: "{autodesktop}\Strategix"; \
Filename: "{app}\{#MyAppExeName}"; \
Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; \
Parameters: "/install /quiet /norestart"; \
StatusMsg: "Installation des composants Microsoft Visual C++..."

Filename: "{app}\{#MyAppExeName}"; \
Description: "Lancer Strategix"; \
Flags: nowait postinstall skipifsilent
