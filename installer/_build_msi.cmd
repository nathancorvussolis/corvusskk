@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd

call _build_doc.cmd

set BUILDCOMMAND=dotnet build installer-msi.wixproj -nologo -verbosity normal

echo build x86.msi
%BUILDCOMMAND% -property:OutputName=x86 -property:InstallerPlatform=x86 -property:BaseIntermediateOutputPath=%OutDir%\x86\

echo build x64.msi
%BUILDCOMMAND% -property:OutputName=x64 -property:InstallerPlatform=x64 -property:BaseIntermediateOutputPath=%OutDir%\x64\

echo build arm64.msi
%BUILDCOMMAND% -property:OutputName=arm64 -property:InstallerPlatform=arm64 -property:BaseIntermediateOutputPath=%OutDir%\arm64\

popd
endlocal
