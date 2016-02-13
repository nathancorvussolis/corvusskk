@echo off
setlocal

pushd "%~dp0"

call "%VS120COMNTOOLS%VsDevCmd.bat"

call _version.cmd



rem > _sign.cmd <URL 1> <pfx file 1> <password 1> <URL 2> <pfx file 2> <password 2>
rem     * <URL 1> : SHA-1 Authenticode timestamp server
rem     * <pfx file 1> : pfx file for SHA-1 file digest algorithm
rem     * <URL 2> : SHA-256 RFC-3161 timestamp server
rem     * <pfx file 2> : pfx file for SHA-256 file digest algorithm



set DESCRIPTION="CorvusSKK"

rem option "/fd sha1 /t <SHA-1 Authenticode timestamp server>"
set TIMESTAMPSERVER1=%1
set PFXFILE1=%2
set PASSWORD1=%3

rem option "/fd sha256 /tr <SHA-2 RFC-3161 timestamp server> /td sha256"
set TIMESTAMPSERVER2=%4
set PFXFILE2=%5
set PASSWORD2=%6

set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe" "..\x64\Release\*.dll" "..\x64\Release\*.exe"
set MSIFILES="%TARGETDIR%\x86.msi" "%TARGETDIR%\x64.msi"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"
set BEFILE="%TARGETDIR%\engine.exe"

set SIGNCOMMAND1=signtool sign /v /d %DESCRIPTION% /f %PFXFILE1% /p %PASSWORD1% /fd sha1 /t %TIMESTAMPSERVER1%
set SIGNCOMMAND2=signtool sign /v /as /d %DESCRIPTION% /f %PFXFILE2% /p %PASSWORD2% /fd sha256 /tr %TIMESTAMPSERVER2% /td sha256
set SIGNCOMMANDMSI=signtool sign /v /d %DESCRIPTION% /f %PFXFILE2% /p %PASSWORD2% /fd sha1 /t %TIMESTAMPSERVER1%



call _clean.cmd

%SIGNCOMMAND1% %BINFILES%
%SIGNCOMMAND2% %BINFILES%

call _build_x86.cmd
call _build_x64.cmd

%SIGNCOMMANDMSI% %MSIFILES%

call _build_bundle.cmd

"%WIX%\bin\insignia.exe" -ib %BSFILE% -o %BEFILE%
%SIGNCOMMAND1% %BEFILE%
%SIGNCOMMAND2% %BEFILE%

"%WIX%\bin\insignia.exe" -ab %BEFILE% %BSFILE% -o %BSFILE%
%SIGNCOMMAND1% %BSFILE%
%SIGNCOMMAND2% %BSFILE%



popd

endlocal
