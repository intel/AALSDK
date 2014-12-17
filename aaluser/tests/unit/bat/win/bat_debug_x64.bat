@echo off

set errorlevel=
call bat_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC" x64 "Debug" x64 %1
if not "%errorlevel%" == "0" goto :end
if not "%1" == "" goto :end

set errorlevel=
call bat_run.bat Debug x64

:end
exit /b %errorlevel%

