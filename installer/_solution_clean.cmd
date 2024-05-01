@echo off
setlocal
pushd "%~dp0"

if exist "..\build" rd /s /q "..\build"

popd
endlocal
