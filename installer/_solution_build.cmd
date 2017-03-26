@echo off
setlocal

pushd "%~dp0"

call "%VS140COMNTOOLS%VsDevCmd.bat"

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=x86
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=x64

popd

endlocal
