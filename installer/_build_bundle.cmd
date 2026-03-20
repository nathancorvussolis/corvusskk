@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd

set BUILDCOMMAND=dotnet build installer-bundle.wixproj -nologo -verbosity normal

echo build bundle
%BUILDCOMMAND% -property:BaseIntermediateOutputPath=%OutDir%\bundle\

popd
endlocal
