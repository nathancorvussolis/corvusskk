@echo off
setlocal

pushd %~dp0

call _version.cmd
set TARGETDIR=build

del "%TARGETDIR%\corvusskk-%VERSION%-x64.*"

popd

endlocal
