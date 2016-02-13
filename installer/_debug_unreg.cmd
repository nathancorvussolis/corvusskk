pushd "%~dp0"
regsvr32 /s /u ..\Win32\Debug\imcrvtip.dll
regsvr32 /s /u ..\x64\Debug\imcrvtip.dll
popd
