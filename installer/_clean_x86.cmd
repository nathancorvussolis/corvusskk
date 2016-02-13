@echo off
setlocal

pushd "%~dp0"

call _version.cmd

del "%TARGETDIR%\README.html"
del "%TARGETDIR%\x86.*"

popd

endlocal
