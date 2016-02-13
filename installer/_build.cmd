@echo off
setlocal

pushd "%~dp0"

call _clean.cmd

call _build_x86.cmd
call _build_x64.cmd

call _build_bundle.cmd

popd

endlocal
