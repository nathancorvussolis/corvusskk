pushd "%~dp0"
regsvr32 /s /u ..\build\Win32\Debug\imcrvtip.dll
regsvr32 /s /u ..\build\x64\Debug\imcrvtip.dll
popd
