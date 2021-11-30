@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd



rem x86
set BINFILES="..\Win32\Release\*.dll" "..\Win32\Release\*.exe"
rem x64
set BINFILES=%BINFILES% "..\x64\Release\*.dll" "..\x64\Release\*.exe"
rem ARM32   TIP only
set BINFILES=%BINFILES% "..\ARM\Release\*.dll"
rem ARM64
set BINFILES=%BINFILES% "..\ARM64\Release\*.dll" "..\ARM64\Release\*.exe"
rem ARM64EC   TIP only
set BINFILES=%BINFILES% "..\ARM64EC\Release\*.dll"

rem x86
set MSIFILES="%TARGETDIR%\x86.msi"
rem x64
set MSIFILES=%MSIFILES% "%TARGETDIR%\x64.msi"
rem ARM
set MSIFILES=%MSIFILES% "%TARGETDIR%\arm.msi"

rem bundle
set BEFILE="%TARGETDIR%\engine.exe"
set BSFILE="%TARGETDIR%\corvusskk-%VERSION%.exe"



signtool verify /all /v /d /pa /tw %BINFILES% %MSIFILES% %BEFILE% %BSFILE%

set SIGNCOUNT=0
for %%i in (%BINFILES% %MSIFILES% %BEFILE% %BSFILE%) do set /a "SIGNCOUNT = SIGNCOUNT + 1"

echo;
echo     %SIGNCOUNT% signatures in all.



popd

endlocal
