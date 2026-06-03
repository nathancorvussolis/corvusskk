@echo off
setlocal
pushd "%~dp0"

call _env.cmd

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

pushd ..

set PATH=%PATH%;%LocalAppData%\Pandoc;%ProgramFiles%\Pandoc

set DESCRIPTION=CorvusSKK

pandoc.exe ^
-s ^
-f gfm-ascii_identifiers ^
-t html5 ^
-V lang:"ja" ^
-V title-prefix:"%DESCRIPTION%" ^
-V pagetitle:"Manual" ^
-V title:"%DESCRIPTION% Manual" ^
--embed-resources ^
--standalone ^
-c "installer\resource-md\markdown.css" ^
--toc ^
-o "installer\%OUTDIR%\README.html" ^
README.md

popd

copy /y /b "..\LICENSE.TXT" "%OUTDIR%\LICENSE.txt" > nul
copy /y /b "config-lua\init.lua" "%OUTDIR%\init.lua" > nul
copy /y /b "config-share\config.xml" "%OUTDIR%\config.xml" > nul
copy /y /b "config-share\skkdict.txt" "%OUTDIR%\skkdict.txt" > nul

pushd "%OUTDIR%"

copy /b LICENSE.txt + > nul
copy /b init.lua + > nul
copy /b config.xml + > nul
copy /b skkdict.txt + > nul

popd

popd
endlocal
