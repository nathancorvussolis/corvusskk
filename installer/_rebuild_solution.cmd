@pushd %~dp0
@echo off

setlocal

call "%VS120COMNTOOLS%VsDevCmd.bat"

msbuild ..\CorvusSKK.sln /nologo /maxcpucount /verbosity:normal /target:Rebuild /property:Configuration=Release,Platform=Win32
msbuild ..\CorvusSKK.sln /nologo /maxcpucount /verbosity:normal /target:Rebuild /property:Configuration=Release,Platform=x64

endlocal

@popd
