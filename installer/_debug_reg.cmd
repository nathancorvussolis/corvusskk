pushd "%~dp0"
regsvr32 /s ..\build\Win32\Debug\imcrvtip.dll
regsvr32 /s ..\build\x64\Debug\imcrvtip.dll
popd
