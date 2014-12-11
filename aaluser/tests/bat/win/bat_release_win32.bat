@echo off

set errorlevel=
call bat_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC" x86 "Release" Win32 %1
if not "%errorlevel%" == "0" goto :end
if not "%1" == "" goto :end

set errorlevel=
call bat_run.bat Release Win32

:end
exit /b %errorlevel%

