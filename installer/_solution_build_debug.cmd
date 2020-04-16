@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

rem x86

msbuild "..\imcrvskk.sln" ^
/nologo /maxcpucount /verbosity:normal /target:Build ^
/property:Configuration=Debug,Platform=x86

rem x64

msbuild "..\imcrvskk.sln" ^
/nologo /maxcpucount /verbosity:normal /target:Build ^
/property:Configuration=Debug,Platform=x64

rem ARM32

msbuild "..\imcrvskk.sln" ^
/nologo /maxcpucount /verbosity:normal /target:Build ^
/property:Configuration=Debug,Platform=ARM

rem ARM64

msbuild "..\imcrvskk.sln" ^
/nologo /maxcpucount /verbosity:normal /target:Build ^
/property:Configuration=Debug,Platform=ARM64

popd

endlocal
