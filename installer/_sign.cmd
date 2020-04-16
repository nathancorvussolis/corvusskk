@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd



rem > _sign.cmd  <SHA-1 hash> <URL>
rem     * <SHA-1 hash> : SHA-1 hash of certificate for SHA-256 file digest algorithm
rem     * <URL> : SHA-256 RFC-3161 timestamp server

set DESCRIPTION="CorvusSKK"

set SHA1HASH=%1
set TIMESTAMPSERVER=%2

rem x86
set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe"
rem x64
set BINFILES=%BINFILES% "..\x64\Release\*.dll" "..\x64\Release\*.exe"
rem ARM32   TIP only
set BINFILES=%BINFILES% "..\ARM\Release\*.dll"
rem ARM64
set BINFILES=%BINFILES% "..\ARM64\Release\*.dll" "..\ARM64\Release\*.exe"

rem x86
set MSIFILES="%TARGETDIR%\x86.msi"
rem x64
set MSIFILES=%MSIFILES% "%TARGETDIR%\x64.msi"
rem ARM
set MSIFILES=%MSIFILES% "%TARGETDIR%\arm.msi"

rem x86/x64
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"

rem ARM
set ARMBEFILE="%TARGETDIR%\engine-arm.exe"
set ARMBSFILE="%TARGETDIR%\corvusskk-%VERSION%-arm.exe"

set SIGNCOMMAND=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH% /fd sha256 /tr %TIMESTAMPSERVER% /td sha256



call _clean.cmd

%SIGNCOMMAND% %BINFILES%

call _build_msi.cmd

%SIGNCOMMAND% %MSIFILES%

call _build_bundle.cmd

rem x86/x64
"%WIX%\bin\insignia.exe" -nologo -ib %BSFILE% -o %BEFILE%

rem ARM
"%WIX%\bin\insignia.exe" -nologo -ib %ARMBSFILE% -o %ARMBEFILE%

%SIGNCOMMAND% %BEFILE% %ARMBEFILE%

rem x86/x64
"%WIX%\bin\insignia.exe" -nologo -ab %BEFILE% %BSFILE% -o %BSFILE%

rem ARM
"%WIX%\bin\insignia.exe" -nologo -ab %ARMBEFILE% %ARMBSFILE% -o %ARMBSFILE%

%SIGNCOMMAND% %BSFILE% %ARMBSFILE%



popd

endlocal
