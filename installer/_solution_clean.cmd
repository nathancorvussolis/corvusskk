@echo off
setlocal

pushd "%~dp0"

call "%VS120COMNTOOLS%VsDevCmd.bat"

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Release,Platform=Win32
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Release,Platform=x64

popd

endlocal
