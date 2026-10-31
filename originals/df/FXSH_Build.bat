@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.


if exist DF.G1A  del DF.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"D:\CASIO\fx-9860G SDK\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"D:\CASIO\fx-9860G SDK\Tools\MakeAddinHeader363.exe" "F:\CASIO\projects\df"
if not exist DF.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

