@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd

call _build_doc.cmd

if not defined SIGNCOMMAND set SignOutput=false

set BUILDCOMMAND=dotnet build installer-msi.wixproj -nologo -verbosity:normal -target:Build

echo build x86.msi
%BUILDCOMMAND% -property:InstallerPlatform=x86 -property:BaseIntermediateOutputPath=%OutDir%\x86\

echo build x64.msi
%BUILDCOMMAND% -property:InstallerPlatform=x64 -property:BaseIntermediateOutputPath=%OutDir%\x64\

echo build arm.msi
%BUILDCOMMAND% -property:InstallerPlatform=arm64 -property:BaseIntermediateOutputPath=%OutDir%\arm\

popd
endlocal
