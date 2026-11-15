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
VersionInfoVersion={#MyAppVersion}
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
Name: "en"; MessagesFile: "compiler:Default.isl"

[Files]
Source: ".\build\README.html";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\LICENSE.txt";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\config.xml";                        DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\init.lua";                          DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}
Source: ".\build\skkdict.txt";                       DestDir: "{win}\{#MyAppDir}"; Flags: {#CommonFileFlags}

Source: "..\build\Win32\Release\imcrvcnf.exe";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\imcrvmgr.exe";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\imcrvtip.dll";       DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\lua.exe";            DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\lua55.dll";          DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "..\build\Win32\Release\zlib1.dll";          DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit

Source: "{#VCDir}x86\{#VCCRT}\msvcp140.dll";         DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit
Source: "{#VCDir}x86\{#VCCRT}\vcruntime140.dll";     DestDir: "{sys}\{#MyAppDir}"; Flags: {#CommonFileFlags} 32bit

Source: "..\build\x64\Release\imcrvcnf.exe";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\imcrvmgr.exe";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\imcrvtip.dll";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\lua.exe";              DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\lua55.dll";            DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "..\build\x64\Release\zlib1.dll";            DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit

Source: "{#VCDir}x64\{#VCCRT}\msvcp140.dll";         DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}x64\{#VCCRT}\vcruntime140.dll";     DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit
Source: "{#VCDir}x64\{#VCCRT}\vcruntime140_1.dll";   DestDir: "{sys}\{#MyAppDir}"; Check: IsX64OS; Flags: {#CommonFileFlags} 64bit

Source: "..\build\ARM64\Release\imcrvcnf.exe";       DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64\Release\imcrvmgr.exe";       DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
Source: "..\build\ARM64EC\Release\imcrvtip.dll";     DestDir: "{sys}\{#MyAppDir}"; Check: IsArm64; Flags: {#CommonFileFlags} 64bit
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
Root: HKLM32; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; \
  ValueType: string; ValueName: "imcrvmgr"; ValueData: """{sys}\{#MyAppDir}\imcrvmgr.exe"""; \
  Flags: uninsdeletevalue; Check: IsX86OS
Root: HKLM64; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; \
  ValueType: string; ValueName: "imcrvmgr"; ValueData: """{sys}\{#MyAppDir}\imcrvmgr.exe"""; \
  Flags: uninsdeletevalue; Check: IsX64OS or IsArm64

; regserverフラグの代替処理
;
; ・インストール/アップグレード
;   1) PrepareToInstallイベントで旧DLLを登録解除, リネーム, 再起動後の削除を予約
;   2) [Files]で新DLLをインストール
;   3) [Run]で新DLLを登録
;   4) 再起動後に旧DLLが削除
;
; ・アンインストール
;   1) [UninstallRun]で登録解除
;   2) 再起動後にuninsrestartdeleteで削除

[Run]
; 登録
Filename: "{sys}\regsvr32.exe"; Parameters: "/s ""{sys}\{#MyAppDir}\imcrvtip.dll"""; \
  Flags: runhidden 32bit
Filename: "{sys}\regsvr32.exe"; Parameters: "/s ""{sys}\{#MyAppDir}\imcrvtip.dll"""; \
  Flags: runhidden 64bit; Check: IsX64OS or IsArm64

[UninstallRun]
; 登録解除
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{sys}\{#MyAppDir}\imcrvtip.dll"""; \
  RunOnceId: "unreg32"; Flags: runhidden 32bit
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{sys}\{#MyAppDir}\imcrvtip.dll"""; \
  RunOnceId: "unreg64"; Flags: runhidden 64bit; Check: IsX64OS or IsArm64

[Code]
// TIP DLL
const
  TipFilePath = '{sys}\{#MyAppDir}\imcrvtip.dll';

// 旧バージョンのDLLを登録解除, リネーム, 再起動後の削除を予約
function UninstallExistingServerDLL(const Is64Bit: Boolean; const FilePath: String): String;
var
  RedirFilePath: String;
  TempFilePath: String;
begin
  Result := ''
  RedirFilePath := ApplyPathRedirRulesForCurrentProcess(Is64Bit, FilePath);
  TempFilePath := GenerateUniqueName(ExtractFileDir(RedirFilePath), '.dll.tmp');

  if not FileExists(RedirFilePath) then
  begin
    Log('Server DLL not found. ' + RedirFilePath);
    Exit;
  end;
  Log('Server DLL found. ' + RedirFilePath);

  // 登録解除
  if UnregisterServer(Is64Bit, RedirFilePath, False) then
  begin
    Log('Successfully unregistered server DLL. ' + RedirFilePath);
  end else begin
    Log('Failed to unregister server DLL. ' + RedirFilePath);
    Result := 'Failed to unregister server DLL. ' + RedirFilePath;
    Exit;
  end;
  // リネーム
  if RenameFile(RedirFilePath, TempFilePath) then
  begin
    Log('Successfully renamed server DLL. ' + TempFilePath);
  end else begin
    Log('Failed to rename server DLL. ' + TempFilePath);
    Result := 'Failed to rename server DLL. ' + TempFilePath;
    Exit;
  end;
  // 再起動後の削除を予約
  try
    RestartReplace(TempFilePath, '');
    Log('Successfully scheduled deletion server DLL. ' + TempFilePath);
  except
    Log('Failed to schedule deletion server DLL. ' + TempFilePath);
    Result := 'Failed to schedule deletion server DLL. ' + TempFilePath;
    Exit;
  end;
end;

// WiXToolsetの旧バージョンインストーラーの情報
const
  // The target bundle is 32bit.
  RegSubKeyUninstall = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall';
  // Bundle UpgradeCode
  TargetBundleUpgradeCode = '{F2664253-EAE9-4ED5-AD92-03229BD8F64F}';

// WiXToolsetの旧バージョンインストーラーを検索
function SearchBundleCachePath(const BundleUpgradeCode: String; var BundleCachePath: String): Boolean;
var
  SubKeyNameArr: TArrayOfString;
  UpgradeCode: String;
  UpgradeCodeArr: TArrayOfString;
  CachePath: String;
  I, J: Integer;
begin
  Result := False;
  if RegGetSubkeyNames(HKLM32, RegSubKeyUninstall, SubKeyNameArr) then
  begin
    for I := 0 to GetArrayLength(SubKeyNameArr) - 1 do
    begin
      if RegQueryMultiStringValue(HKLM32, RegSubKeyUninstall + '\' + SubKeyNameArr[I], 'BundleUpgradeCode', UpgradeCode) then
      begin
        UpgradeCodeArr := StringSplit(UpgradeCode, [#0], stExcludeEmpty);
        for J := 0 to GetArrayLength(UpgradeCodeArr) - 1 do
        begin
          if SameText(UpgradeCodeArr[J], BundleUpgradeCode) then
          begin
            if RegQueryStringValue(HKLM32, RegSubKeyUninstall + '\' + SubKeyNameArr[I], 'BundleCachePath', CachePath) then
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
end;

// WiXToolsetの旧バージョンインストーラーをアンインストール
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
    begin
      Log('Successfully uninstalled Bundle. Code=' + IntToStr(ResultCode));
    end else begin
      Log('Failed to uninstall Bundle. Code=' + IntToStr(ResultCode));
    end;
  end else begin
    Log('Bundle not found.');
  end;
end;

// イベント
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';

  // Uninstall previous versions of bundle
  UninstallBundle(TargetBundleUpgradeCode);

  // Uninstall existing 32bit DLL
  Result := UninstallExistingServerDLL(False, ExpandConstant(TipFilePath));
  if Result <> '' then Exit;

  // Uninstall existing 64bit DLL
  if IsX64OS or IsArm64 then
  begin
    Result := UninstallExistingServerDLL(True, ExpandConstant(TipFilePath));
    if Result <> '' then Exit;
  end;
end;

// WiXToolsetの旧バージョンインストーラーの検索結果をテスト用に表示
#if False
procedure SearchBundleTest(const BundleUpgradeCode: String);
var
  CachePath: String;
begin
  if SearchBundleCachePath(BundleUpgradeCode, CachePath) then
  begin
    MsgBox('Bundle found.' + #13#10#13#10 +
      'BundleUpgradeCode :' + #13#10 + BundleUpgradeCode + #13#10#13#10 +
      'BundleCachePath :' + #13#10 + CachePath,
      mbInformation, MB_OK);
  end else begin
    MsgBox('Bundle not found.' + #13#10#13#10 +
      'BundleUpgradeCode :' + #13#10 + BundleUpgradeCode,
      mbError, MB_OK);
  end;
end;

// イベント
function InitializeSetup: Boolean;
begin
  Result := False;
  SearchBundleTest(TargetBundleUpgradeCode);
end;
#endif
