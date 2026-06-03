#define MyAppName "CorvusSKK"
#define MyAppVersion GetEnv('VERSION')
#define MyAppPublisher "nathancorvussolis"
#define MyAppURL "https://nathancorvussolis.github.io/"
#define MyAppDir "IME\IMCRVSKK"
#define MyOutDir GetEnv('OUTDIR')

; Visual C++ Redistributable DLLs
#define VCDir GetEnv('VCToolsRedistDir')
#define VCCRT "Microsoft.VC143.CRT"
; Common options for file flags
#define CommonFileFlags "ignoreversion restartreplace uninsrestartdelete"

[Setup]
AppId={{F2664253-EAE9-4ED5-AD92-03229BD8F64F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppCopyright=© 2011 {#MyAppPublisher}
VersionInfoVersion={#MyAppVersion}.0
OutputDir={#MyOutDir}
OutputBaseFilename=corvusskk-{#MyAppVersion}
SetupIconFile=compiler:SetupClassicIcon.ico
SolidCompression=yes
; Sign
#ifdef SIGN
SignTool=MySignTool
SignTool=MyCopyTool
SignedUninstaller=yes
SignToolRetryCount=0
#endif
; Windows 10 version 1607 (build 14393) or later
MinVersion=10.0.14393
; Elevated rights
PrivilegesRequired=admin
; Wizard
WizardStyle=classic
LicenseFile=license.rtf
DisableDirPage=yes
DisableProgramGroupPage=yes
; Log
SetupLogging=yes
UninstallLogging=yes
; Files in use
CloseApplications=no
RestartApplications=no
; Restart
AlwaysRestart=yes
UninstallRestartComputer=yes
; Default directory
DefaultDirName={sys}\{#MyAppDir}
; Start Menu
DefaultGroupName={#MyAppName}
; Uninstall entry
UninstallDisplayIcon="{sys}\{#MyAppDir}\imcrvtip.dll",-100

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: ".\build\README.html";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\LICENSE.txt";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\config.xml";                        DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\init.lua";                          DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\skkdict.txt";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}

Source: "..\build\Win32\Release\imcrvcnf.exe";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\imcrvmgr.exe";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\imcrvtip.dll";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit regserver
Source: "..\build\Win32\Release\lua.exe";            DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\lua55.dll";          DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\zlib1.dll";          DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit

Source: "{#VCDir}x86\{#VCCRT}\msvcp140.dll";         DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "{#VCDir}x86\{#VCCRT}\vcruntime140.dll";     DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit

Source: "..\build\x64\Release\imcrvcnf.exe";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\imcrvmgr.exe";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\imcrvtip.dll";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit regserver
Source: "..\build\x64\Release\lua.exe";              DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\lua55.dll";            DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\zlib1.dll";            DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit

Source: "{#VCDir}x64\{#VCCRT}\msvcp140.dll";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}x64\{#VCCRT}\vcruntime140.dll";     DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}x64\{#VCCRT}\vcruntime140_1.dll";   DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit

Source: "..\build\ARM64\Release\imcrvcnf.exe";       DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64\Release\imcrvmgr.exe";       DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64EC\Release\imcrvtip.dll";     DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit regserver
Source: "..\build\ARM64\Release\lua.exe";            DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64\Release\lua55.dll";          DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64\Release\zlib1.dll";          DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit

Source: "{#VCDir}arm64\{#VCCRT}\msvcp140.dll";       DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}arm64\{#VCCRT}\vcruntime140.dll";   DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}arm64\{#VCCRT}\vcruntime140_1.dll"; DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit

[Icons]
Name: "{group}\CONFIG";  Filename: "{sys}\{#MyAppDir}\imcrvcnf.exe"
Name: "{group}\README";  Filename: "{win}\{#MyAppDir}\README.html"
Name: "{group}\LICENSE"; Filename: "{win}\{#MyAppDir}\LICENSE.txt"

[Registry]
Root: HKLM32; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; \
  ValueName: "imcrvmgr"; ValueData: """{sys}\{#MyAppDir}\imcrvmgr.exe"""; Check: IsX86OS; Flags: uninsdeletevalue
Root: HKLM64; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; \
  ValueName: "imcrvmgr"; ValueData: """{sys}\{#MyAppDir}\imcrvmgr.exe"""; Check: IsWin64; Flags: uninsdeletevalue

[Code]
const
  // Both the target bundle and itself are 32bit. This key is redirected to WOW6432Node on 64bit OS.
  RegSubKeyUninstall = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall';
  // Bundle UpgradeCode
  TargetBundleUpgradeCode = '{F2664253-EAE9-4ED5-AD92-03229BD8F64F}';

function MultiStringContains(const MultiStr: String; const TargetStr: String): Boolean;
var
  MultiStrArr: TArrayOfString;
  I: Integer;
begin
  Result := False;
  MultiStrArr := StringSplit(MultiStr, [#0], stExcludeEmpty);
  for I := 0 to GetArrayLength(MultiStrArr) - 1 do
  begin
    if SameText(MultiStrArr[I], TargetStr) then
    begin
      Result := True;
      Exit;
    end;
  end;
end;

function SearchBundleCachePath(const BundleUpgradeCode: String; var BundleCachePath: String): Boolean;
var
  SubKeys: TArrayOfString;
  UpgradeCode: String;
  CachePath: String;
  I: Integer;
begin
  Result := False;
  if RegGetSubkeyNames(HKLM, RegSubKeyUninstall, SubKeys) then
  begin
    for I := 0 to GetArrayLength(SubKeys) - 1 do
    begin
      if RegQueryMultiStringValue(HKLM, RegSubKeyUninstall + '\' + SubKeys[I], 'BundleUpgradeCode', UpgradeCode) then
      begin
        if MultiStringContains(UpgradeCode, BundleUpgradeCode) then
        begin
          if RegQueryStringValue(HKLM, RegSubKeyUninstall + '\' + SubKeys[I], 'BundleCachePath', CachePath) then
          begin
            if FileExists(CachePath) then
            begin
              BundleCachePath := CachePath;
              Result := True;
              Exit;
            end;
          end;
        end;
      end;
    end;
  end;
end;

procedure UninstallBundle(const BundleUpgradeCode: String);
var
  CachePath: String;
  ResultCode: Integer;
begin
  Log('Searching Bundle. BundleUpgradeCode : ' + BundleUpgradeCode);
  if SearchBundleCachePath(BundleUpgradeCode, CachePath) then
  begin
    Log('Bundle found. BundleCachePath : ' + CachePath);
    Log('Uninstalling Bundle.');
    if Exec(CachePath, '/uninstall /quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
      Log('Succeded to uninstall Bundle. Code=' + IntToStr(ResultCode))
    else
      Log('Failed to uninstall Bundle. Code=' + IntToStr(ResultCode));
  end
  else
  begin
    Log('Bundle not found.');
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  // Uninstall previous versions of bundle
  UninstallBundle(TargetBundleUpgradeCode);
  Result := '';
end;
