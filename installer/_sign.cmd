@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd



rem > _sign.cmd  <SHA-1 has> <URL>
rem     * <SHA-1 hash> : SHA-1 hash of certificate for SHA-256 file digest algorithm
rem     * <URL> : SHA-256 RFC-3161 timestamp server



set DESCRIPTION="CorvusSKK"

rem option "/fd sha256 /tr <SHA-2 RFC-3161 timestamp server> /td sha256"
set SHA1HASH=%1
set TIMESTAMPSERVER=%2

set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe" "..\x64\Release\*.dll" "..\x64\Release\*.exe"
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  set BINFILES=%BINFILES% "..\ARM\Release\*.dll" "..\ARM64\Release\*.dll" "..\ARM64\Release\*.exe"
)
set MSIFILES="%TARGETDIR%\x86.msi" "%TARGETDIR%\x64.msi"
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  set MSIFILES=%MSIFILES% "%TARGETDIR%\arm.msi"
)
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"
set ARMBEFILE=
set ARMBSFILE=
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  set ARMBEFILE="%TARGETDIR%\engine-arm.exe"
  set ARMBSFILE="%TARGETDIR%\corvusskk-%VERSION%-arm.exe"
)

set SIGNCOMMAND=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH% /fd sha256 /tr %TIMESTAMPSERVER% /td sha256



call _clean.cmd

%SIGNCOMMAND% %BINFILES%

call _build_msi.cmd

%SIGNCOMMAND% %MSIFILES%

call _build_bundle.cmd

"%WIX%\bin\insignia.exe" -nologo -ib %BSFILE% -o %BEFILE%
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  "%WIX%\bin\insignia.exe" -nologo -ib %ARMBSFILE% -o %ARMBEFILE%
)
%SIGNCOMMAND% %BEFILE% %ARMBEFILE%

"%WIX%\bin\insignia.exe" -nologo -ab %BEFILE% %BSFILE% -o %BSFILE%
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  "%WIX%\bin\insignia.exe" -nologo -ab %ARMBEFILE% %ARMBSFILE% -o %ARMBSFILE%
)
%SIGNCOMMAND% %BSFILE% %ARMBSFILE%



popd

endlocal
