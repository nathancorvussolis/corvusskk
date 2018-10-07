@echo off
setlocal

pushd "%~dp0"

call "%VS140COMNTOOLS%vsvars32.bat" 8.1

call _version.cmd



set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe" "..\x64\Release\*.dll" "..\x64\Release\*.exe"
set MSIFILES="%TARGETDIR%\x86.msi" "%TARGETDIR%\x64.msi"
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"



signtool verify /all /v /d /pa /tw %BINFILES% %MSIFILES% %BEFILE% %BSFILE%

set SIGNCOUNT=0
for %%i in (%BINFILES% %BEFILE% %BSFILE%) do set /a "SIGNCOUNT = SIGNCOUNT + 2"
for %%i in (%MSIFILES%) do set /a "SIGNCOUNT = SIGNCOUNT + 1"

echo;
echo     %SIGNCOUNT% signatures in all.



popd

endlocal
