@echo off
setlocal

pushd %~dp0

call _build_x86.cmd
call _build_x64.cmd

popd

endlocal
