@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd



rem > _sign.cmd  <SHA-1 hash 1> <URL 1> <SHA-1 hash 2> <URL 2>
rem     * <SHA-1 hash 1> : SHA-1 hash of certificate for SHA-1 file digest algorithm
rem     * <URL 1> : SHA-1 Authenticode timestamp server
rem     * <SHA-1 hash 2> : SHA-1 hash of certificate for SHA-256 file digest algorithm
rem     * <URL 2> : SHA-256 RFC-3161 timestamp server



set DESCRIPTION="CorvusSKK"

rem option "/fd sha1 /t <SHA-1 Authenticode timestamp server>"
set SHA1HASH1=%1
set TIMESTAMPSERVER1=%2

rem option "/fd sha256 /tr <SHA-2 RFC-3161 timestamp server> /td sha256"
set SHA1HASH2=%3
set TIMESTAMPSERVER2=%4

set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe" "..\x64\Release\*.dll" "..\x64\Release\*.exe"
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  set BINFILES=%BINFILES% "..\ARM\Release\*.dll" "..\ARM\Release\*.exe" "..\ARM64\Release\*.dll" "..\ARM64\Release\*.exe"
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

set SIGNCOMMAND1=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH1% /fd sha1 /t %TIMESTAMPSERVER1%
set SIGNCOMMAND2=signtool sign /v /as /d %DESCRIPTION% /sha1 %SHA1HASH2% /fd sha256 /tr %TIMESTAMPSERVER2% /td sha256
set SIGNCOMMANDMSI=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH2% /fd sha1 /t %TIMESTAMPSERVER1%



call _clean.cmd

%SIGNCOMMAND1% %BINFILES%
%SIGNCOMMAND2% %BINFILES%

call _build_msi.cmd

%SIGNCOMMANDMSI% %MSIFILES%

call _build_bundle.cmd

"%WIX%\bin\insignia.exe" -nologo -ib %BSFILE% -o %BEFILE%
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  "%WIX%\bin\insignia.exe" -nologo -ib %ARMBSFILE% -o %ARMBEFILE%
)
%SIGNCOMMAND1% %BEFILE% %ARMBEFILE%
%SIGNCOMMAND2% %BEFILE% %ARMBEFILE%

"%WIX%\bin\insignia.exe" -nologo -ab %BEFILE% %BSFILE% -o %BSFILE%
if "%ENABLE_PLATFORM_ARM%" neq "0" (
  "%WIX%\bin\insignia.exe" -nologo -ab %ARMBEFILE% %ARMBSFILE% -o %ARMBSFILE%
)
%SIGNCOMMAND1% %BSFILE% %ARMBSFILE%
%SIGNCOMMAND2% %BSFILE% %ARMBSFILE%



popd

endlocal
