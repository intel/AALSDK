@echo off

set GTEST=gtest-1.7.0
set  GZIP=gzip.exe
set   TAR=tar.exe


set errorlevel=0
if exist %GTEST% goto success


if not exist "%GTEST%.tar.gz" goto gtest_zip_error

copy "%GTEST%.tar.gz" "temp-%GTEST%.tar.gz"

set errorlevel=
.\%GZIP% -d "temp-%GTEST%.tar.gz"
rem if %errorlevel% neq 0 goto gzip_error

if not exist "temp-%GTEST%.tar" goto gtest_tar_error

set errorlevel=
.\%TAR% -x -f "temp-%GTEST%.tar"
rem if %errorlevel% neq 0 goto tar_error

if not exist "%GTEST%" goto gtest_error

del "temp-%GTEST%.tar" 
echo %GTEST% decompressed.

:success
exit /b %errorlevel%


:gtest_zip_error
echo %GTEST%.tar.gz not found.
set errorlevel=2
goto error

:gzip_error
echo %GZIP% failed.
set errorlevel=3
goto error

:gtest_tar_error
echo temp-%GTEST%.tar not found.
set errorlevel=4
goto error

:tar_error
echo %TAR% failed.
set errorlevel=5
goto error

:gtest_error
echo %GTEST% not found.
set errorlevel=6
goto error

:error
exit /b %errorlevel%
