@echo off
set errorlevel=
call aalsdk_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC" x64 "Win8.1 Release" x64 %1
exit /b %errorlevel%
