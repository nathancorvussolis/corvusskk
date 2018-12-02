@echo off
setlocal

pushd "%~dp0"

call _clean.cmd

call _build_msi.cmd

call _build_bundle.cmd

popd

endlocal
