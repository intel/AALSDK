@echo off

if "%1" == "" goto missing_configuration
if "%2" == "" goto missing_platform

set errorlevel=
..\win\%1\%2\bin\bat.exe
if not "%errorlevel%" == "0" goto err

set errorlevel=
..\win\%1\%2\bin\XLSample1.exe
if not "%errorlevel%" == "0" goto err

set errorlevel=
..\win\%1\%2\bin\XLSample2.exe < bat_run.bat | ..\win\%1\%2\bin\XLSample2.exe 
if not "%errorlevel%" == "0" goto err

set errorlevel=
..\win\%1\%2\bin\cciapp.exe --target=swsim
if not "%errorlevel%" == "0" goto err

set errorlevel=
..\win\%1\%2\bin\splapp.exe --target=swsim
if not "%errorlevel%" == "0" goto err

exit /b %errorlevel%


:missing_configuration
set errorlevel=1
echo Give the build Configuration (Debug or Release) as the first parameter.
goto :err

:missing_platform
set errorlevel=2
echo Give x64 or Win32 as the second parameter. (code generation target)
goto :err


:err
exit /b %errorlevel%

