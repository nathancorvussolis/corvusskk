@echo off
setlocal

pushd "%~dp0"

call "%VS140COMNTOOLS%VsDevCmd.bat"

call _version.cmd



set BINFILESX86="..\Win32\Release\*.dll" "..\Win32\Release\*.exe"
set MSIFILESX86="%TARGETDIR%\x86.msi"
set BINFILESX64="..\x64\Release\*.dll" "..\x64\Release\*.exe"
set MSIFILESX64="%TARGETDIR%\x64.msi"
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"



signtool verify /all /v /d /pa /tw %BINFILESX86% %MSIFILESX86% %BINFILESX64% %MSIFILESX64% %BEFILE% %BSFILE%

set SIGNCOUNT=0
for %%i in (%BINFILESX86% %BINFILESX64% %BEFILE% %BSFILE%) do set /a "SIGNCOUNT = SIGNCOUNT + 2"
for %%i in (%MSIFILESX86% %MSIFILESX64%) do set /a "SIGNCOUNT = SIGNCOUNT + 1"

echo;
echo     %SIGNCOUNT% signatures in all.



popd

endlocal
