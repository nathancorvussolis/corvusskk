@echo off
setlocal
pushd "%~dp0"

call _clean.cmd

call _vsdev.cmd

call _env.cmd

call _build_doc.cmd

set PATH=%PATH%;%ProgramFiles%\Inno Setup 7
set PATH=%PATH%;%ProgramFiles(x86)%\Inno Setup 7
set PATH=%PATH%;%ProgramFiles(x86)%\Inno Setup 6

ISCC.exe installer.iss

popd
endlocal
