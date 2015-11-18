@echo off
setlocal

pushd %~dp0

set TARGETDIR=build

del "%TARGETDIR%\*.wixobj"
del "%TARGETDIR%\*.wixpdb"
del "%TARGETDIR%\*.msi"
del "%TARGETDIR%\*.zip"

popd

endlocal
