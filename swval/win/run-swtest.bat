@echo off

set errorlevel=

if not exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC" goto vc12
call aalsdk_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC" x64 "Debug" x64 %1
if '%errorlevel%' == '0' goto runtests
goto err

:vc12
call aalsdk_build.bat "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC" x64 "Debug" x64 %1
if '%errorlevel%' == '0' goto runtests
goto err


:runtests
cd winbuild\Debug\x64\bin
.\swtest.exe --gtest_print_time=0 --gtest_shuffle %*

:err
exit /b %errorlevel%
