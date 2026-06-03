@echo off
setlocal
pushd "%~dp0"

call _env.cmd

if exist "%OUTDIR%" rd /s /q "%OUTDIR%"

popd
endlocal
