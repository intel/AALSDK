@echo off
set errorlevel=
call aalsdk_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC" x86 "Win7 Debug" Win32 %1
exit /b %errorlevel%
