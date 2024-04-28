@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd

if not defined SIGNCOMMAND set SignOutput=false

set BUILDCOMMAND=dotnet build installer-bundle.wixproj -nologo -verbosity:normal -target:Build

echo build bundle
%BUILDCOMMAND% -property:BaseIntermediateOutputPath=%OutDir%\bundle\

popd
endlocal
