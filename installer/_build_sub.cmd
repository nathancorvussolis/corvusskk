@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if not exist "%TARGETDIR%" mkdir "%TARGETDIR%"

pushd ..

set DESCRIPTION=CorvusSKK

"%LocalAppData%\Pandoc\pandoc.exe" -s -f markdown_github-ascii_identifiers -t html5 -V lang:"ja" -V title-prefix:"%DESCRIPTION%" -V pagetitle:"Manual" -V title:"%DESCRIPTION% Manual" --self-contained -c "installer\resource-md\markdown.css" --columns=1024 --toc -o "installer\%TARGETDIR%\README.html" README.md

popd

copy /y /b "..\LICENSE.TXT" "%TARGETDIR%\LICENSE.txt" > nul
copy /y /b "config-lua\init.lua" "%TARGETDIR%\init.lua" > nul
copy /y /b "config-share\config.xml" "%TARGETDIR%\config.xml" > nul
copy /y /b "config-share\skkdict.txt" "%TARGETDIR%\skkdict.txt" > nul

pushd "%TARGETDIR%"

copy /b LICENSE.txt + > nul
copy /b init.lua + > nul
copy /b config.xml + > nul
copy /b skkdict.txt + > nul

popd

popd

endlocal
