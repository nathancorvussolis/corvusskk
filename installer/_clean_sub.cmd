@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if exist "%TARGETDIR%\README.html" del "%TARGETDIR%\README.html"
if exist "%TARGETDIR%\LICENSE.txt" del "%TARGETDIR%\LICENSE.txt"
if exist "%TARGETDIR%\config.xml" del "%TARGETDIR%\config.xml"
if exist "%TARGETDIR%\init.lua" del "%TARGETDIR%\init.lua"
if exist "%TARGETDIR%\skkdict.txt" del "%TARGETDIR%\skkdict.txt"

popd

endlocal
