@echo off
setlocal

pushd "%~dp0"

call _version.cmd

call _clean_x86.cmd
call _clean_x64.cmd

if exist "%TARGETDIR%\*.exe" del "%TARGETDIR%\*.exe"
if exist "%TARGETDIR%\*.wixobj" del "%TARGETDIR%\*.wixobj"
if exist "%TARGETDIR%\*.wixpdb" del "%TARGETDIR%\*.wixpdb"

popd

endlocal
