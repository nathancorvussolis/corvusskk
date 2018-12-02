@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if exist "%TARGETDIR%\README.html" del "%TARGETDIR%\README.html"
if exist "%TARGETDIR%\LICENSE.txt" del "%TARGETDIR%\LICENSE.txt"
if exist "%TARGETDIR%\config.xml" del "%TARGETDIR%\config.xml"
if exist "%TARGETDIR%\init.lua" del "%TARGETDIR%\init.lua"
if exist "%TARGETDIR%\skkdict.txt" del "%TARGETDIR%\skkdict.txt"

if exist "%TARGETDIR%\*.exe" del "%TARGETDIR%\*.exe"
if exist "%TARGETDIR%\*.msi" del "%TARGETDIR%\*.msi"
if exist "%TARGETDIR%\*.wixobj" del "%TARGETDIR%\*.wixobj"
if exist "%TARGETDIR%\*.wixpdb" del "%TARGETDIR%\*.wixpdb"

popd

endlocal
