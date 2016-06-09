@echo off
setlocal

pushd "%~dp0"

call "%VS140COMNTOOLS%VsDevCmd.bat"

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Debug,Platform=Win32
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Debug,Platform=x64

popd

endlocal
