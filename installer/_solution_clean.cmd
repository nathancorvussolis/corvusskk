@echo off
setlocal

pushd %~dp0

call "%VS120COMNTOOLS%VsDevCmd.bat"

msbuild "..\CorvusSKK.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Release,Platform=Win32
msbuild "..\CorvusSKK.sln" /nologo /maxcpucount /verbosity:normal /target:Clean /property:Configuration=Release,Platform=x64

popd

endlocal
