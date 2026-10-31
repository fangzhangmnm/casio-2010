@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.


if exist VISUALFX.G1A  del VISUALFX.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"D:\CASIO\fx-9860G SDK\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"D:\CASIO\fx-9860G SDK\Tools\MakeAddinHeader363.exe" "F:\CASIO\projects\VisualFX\VisualFX"
if not exist VISUALFX.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

