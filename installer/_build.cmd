@echo off
setlocal

pushd %~dp0

call _clean_x86.cmd
call _clean_x64.cmd

call _build_x86.cmd
call _build_x64.cmd

popd

endlocal
