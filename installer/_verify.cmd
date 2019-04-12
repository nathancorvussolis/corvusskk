@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd



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



signtool verify /all /v /d /pa /tw %BINFILES% %MSIFILES% %BEFILE% %BSFILE% %ARMBEFILE% %ARMBSFILE%

set SIGNCOUNT=0
for %%i in (%BINFILES% %MSIFILES% %BEFILE% %BSFILE% %ARMBEFILE% %ARMBSFILE%) do set /a "SIGNCOUNT = SIGNCOUNT + 1"

echo;
echo     %SIGNCOUNT% signatures in all.



popd

endlocal
