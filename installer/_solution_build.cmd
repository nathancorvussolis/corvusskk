@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=x86
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=x64

msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=ARM
msbuild "..\imcrvskk.sln" /nologo /maxcpucount /verbosity:normal /target:Build /property:Configuration=Release,Platform=ARM64

popd

endlocal
