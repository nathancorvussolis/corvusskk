pushd "%~dp0"
regsvr32 /s ..\Win32\Debug\imcrvtip.dll
regsvr32 /s ..\x64\Debug\imcrvtip.dll
popd
