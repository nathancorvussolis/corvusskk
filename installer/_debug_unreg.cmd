@echo off
setlocal
pushd "%~dp0"

openfiles > nul 2>&1
if %errorlevel% equ 1 (
  powershell start-process "%~nx0" -verb runas
) else (
  regsvr32 /s /u "..\build\Win32\Debug\imcrvtip.dll"
  regsvr32 /s /u "..\build\x64\Debug\imcrvtip.dll"
)

popd
endlocal
