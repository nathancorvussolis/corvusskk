@echo off
setlocal
pushd "%~dp0"

call _env.cmd

if not exist "%OutDir%" mkdir "%OutDir%"

pushd "%OutDir%"

del /q *

if exist x86 rd /s /q x86
if exist x64 rd /s /q x64
if exist arm rd /s /q arm

if exist bundle rd /s /q bundle

popd

popd
endlocal
