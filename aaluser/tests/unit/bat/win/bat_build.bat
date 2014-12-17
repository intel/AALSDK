@echo off

set GTEST_VER=1.6.0

if not exist "..\..\gtest\gtest" goto setup_gtest
goto :get_params

:setup_gtest
cd ..\..\gtest
copy gtest-%GTEST_VER%.tar.gz gtest-%GTEST_VER%.tar.gz.save
.\gzip -d gtest-%GTEST_VER%.tar.gz
.\tar xf gtest-%GTEST_VER%.tar
move gtest-%GTEST_VER% gtest
del gtest-%GTEST_VER%.tar
move gtest-%GTEST_VER%.tar.gz.save gtest-%GTEST_VER%.tar.gz
cd ..\tests\bat

:get_params
set VCINSTALLDIR=%1
set VCINSTALLDIR=%VCINSTALLDIR:"=%

set TOOLBITS=%2

set BUILDCONFIG=%3
set BUILDCONFIG=%BUILDCONFIG:"=%
set BUILDCONFIG=%BUILDCONFIG:'=%
set BUILDCONFIG=%BUILDCONFIG:^ = %

set BUILDPLAT=%4

set BUILDTARGET=%5

if "%VCINSTALLDIR%" == "" goto missing_vcinstall
if "%TOOLBITS%"     == "" goto missing_toolbits
if "%BUILDCONFIG%"  == "" goto missing_configuration
if "%BUILDPLAT%"    == "" goto missing_platform

if not exist "%VCINSTALLDIR%\vcvarsall.bat" goto no_vcvarsall


call "%VCINSTALLDIR%\vcvarsall.bat" %TOOLBITS%
set errorlevel=
msbuild.exe .\bat.sln /p:Configuration="%BUILDCONFIG%" /p:Platform=%BUILDPLAT% %BUILDTARGET%
exit /b %errorlevel%


:missing_vcinstall
set errorlevel=2
echo Give the full path to the VC installation as the first parameter.
echo eg C:\PROGRA~2\MICROS~2.0\VC
goto :err

:missing_toolbits
set errorlevel=3
echo Give x86 or x64 as the second parameter. (32- or 64-bit version of tools)
goto :err

:missing_configuration
set errorlevel=4
echo Give the build Configuration as the third parameter.
echo eg "Win8.1 Debug"
goto :err

:missing_platform
set errorlevel=5
echo Give x64 or Win32 as the fourth parameter. (code generation target)
goto :err

:no_vcvarsall
set errorlevel=6
echo vcvarsall.bat not found at "%VCINSTALLDIR%"
goto :err


:err
exit /b %errorlevel%

