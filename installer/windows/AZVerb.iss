; AZ VERB — Windows o'rnatuvchisi (Inno Setup 6)
#define AppName "AZ VERB"
#define AppVer  "1.1.0"
#define Root    "..\..\build\AZVerb_artefacts\Release"

[Setup]
AppId={{39712DAF-B10C-4AB4-A4AE-30660AE23AB8}
AppName={#AppName}
AppVersion={#AppVer}
AppPublisher=Azimxodjayev Music
DefaultDirName={commonpf64}\Azimxodjayev Music\AZ VERB
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputBaseFilename=AZ-VERB-{#AppVer}-Windows-Setup
Compression=lzma2/ultra
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "To'liq o'rnatish"
Name: "custom"; Description: "Tanlab o'rnatish"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 (Cubase, FL Studio, Ableton, Studio One, Reaper...)"; Types: full custom
Name: "aax";  Description: "AAX (Pro Tools)"; Types: full custom; Check: AaxBuilt
Name: "app";  Description: "Standalone ilova"; Types: full custom

[Files]
Source: "{#Root}\VST3\AZ VERB.vst3\*"; DestDir: "{commoncf64}\VST3\AZ VERB.vst3"; Components: vst3; Flags: recursesubdirs ignoreversion
Source: "{#Root}\AAX\AZ VERB.aaxplugin\*"; DestDir: "{commoncf64}\Avid\Audio\Plug-Ins\AZ VERB.aaxplugin"; Components: aax; Flags: recursesubdirs ignoreversion skipifsourcedoesntexist
Source: "{#Root}\Standalone\AZ VERB.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\AZ VERB"; Filename: "{app}\AZ VERB.exe"; Components: app

[Code]
function AaxBuilt: Boolean;
begin
  Result := True;
end;
