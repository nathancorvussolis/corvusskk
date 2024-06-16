@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd

set BUILDCOMMAND=msbuild "..\imcrvskk.sln" -nologo -maxcpucount -verbosity:normal
set BUILDCOMMAND=%BUILDCOMMAND% -target:Build -property:Configuration=Debug

rem x86
%BUILDCOMMAND% -property:Platform=x86

rem x64
%BUILDCOMMAND% -property:Platform=x64

rem ARM64
%BUILDCOMMAND% -property:Platform=ARM64

rem ARM64EC
%BUILDCOMMAND% -property:Platform=ARM64EC

popd
endlocal
