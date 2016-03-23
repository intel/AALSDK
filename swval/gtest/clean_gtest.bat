@echo off

set GTEST=gtest-1.7.0

if not exist "temp-%GTEST%.tar" goto check_gtest
echo Removing temp-%GTEST%.tar
del /Q "temp-%GTEST%.tar"

:check_gtest
if not exist "%GTEST%" goto success
echo Removing %GTEST%
rd /Q /S "%GTEST%"

:success
exit /b 0

