@echo off
setlocal

pushd "%~dp0"

call "%VS140COMNTOOLS%VsDevCmd.bat"

call _version.cmd



rem > _sign.cmd <URL 1> <SHA-1 hash 1> <URL 2> <SHA-1 hash 2>
rem     * <URL 1> : SHA-1 Authenticode timestamp server
rem     * <SHA-1 hash 1> : SHA-1 hash of certificate for SHA-1 file digest algorithm
rem     * <URL 2> : SHA-256 RFC-3161 timestamp server
rem     * <SHA-1 hash 2> : SHA-1 hash of certificate for SHA-256 file digest algorithm



set DESCRIPTION="CorvusSKK"

rem option "/fd sha1 /t <SHA-1 Authenticode timestamp server>"
set TIMESTAMPSERVER1=%1
set SHA1HASH1=%2

rem option "/fd sha256 /tr <SHA-2 RFC-3161 timestamp server> /td sha256"
set TIMESTAMPSERVER2=%3
set SHA1HASH2=%4

set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe" "..\x64\Release\*.dll" "..\x64\Release\*.exe"
set MSIFILES="%TARGETDIR%\x86.msi" "%TARGETDIR%\x64.msi"
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"

set SIGNCOMMAND1=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH1% /fd sha1 /t %TIMESTAMPSERVER1%
set SIGNCOMMAND2=signtool sign /v /as /d %DESCRIPTION% /sha1 %SHA1HASH2% /fd sha256 /tr %TIMESTAMPSERVER2% /td sha256
set SIGNCOMMANDMSI=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH2% /fd sha1 /t %TIMESTAMPSERVER1%



call _clean.cmd

%SIGNCOMMAND1% %BINFILES%
%SIGNCOMMAND2% %BINFILES%

call _build_x86.cmd
call _build_x64.cmd

%SIGNCOMMANDMSI% %MSIFILES%

call _build_bundle.cmd

"%WIX%\bin\insignia.exe" -nologo -ib %BSFILE% -o %BEFILE%
%SIGNCOMMAND1% %BEFILE%
%SIGNCOMMAND2% %BEFILE%

"%WIX%\bin\insignia.exe" -nologo -ab %BEFILE% %BSFILE% -o %BSFILE%
%SIGNCOMMAND1% %BSFILE%
%SIGNCOMMAND2% %BSFILE%



popd

endlocal
