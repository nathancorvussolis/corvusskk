@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Debug,Platform=x86
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Debug,Platform=x64

if "%ENABLE_PLATFORM_ARM%" neq "0" (
  msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Debug,Platform=ARM
  msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Debug,Platform=ARM64
)

popd

endlocal
