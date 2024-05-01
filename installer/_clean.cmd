@echo off
setlocal
pushd "%~dp0"

call _env.cmd

if exist "%OutDir%" rd /s /q "%OutDir%"

popd
endlocal
