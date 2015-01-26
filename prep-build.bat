@echo off

rem Make sure we're in the right dir.
if not exist "aaluser" goto missing_aaluser
if not exist "aalkernel" goto missing_aalkernel
if not exist "common_hdrs" goto missing_common_hdrs

rem Create the symlinks to common_hdrs if they don't exist.
if exist "aaluser\include\aalsdk\kernel" goto aalkernel_symlink
cd "aaluser\include\aalsdk"

set errorlevel=
mklink /d kernel "..\..\..\common_hdrs"
if not "%errorlevel%" == "0" goto mklink_failed

cd "..\..\.."

:aalkernel_symlink
if exist "aalkernel\include\aalsdk\kernel" goto done
cd "aalkernel\include\aalsdk"

set errorlevel=
mklink /d kernel "..\..\..\common_hdrs"
if not "%errorlevel%" == "0" goto mklink_failed

cd "..\..\.."

:done
exit /b 0


:missing_aaluser
set errorlevel=2
echo aaluser not found.
goto :err

:missing_aalkernel
set errorlevel=3
echo aalkernel not found.
goto :err

:missing_common_hdrs
set errorlevel=4
echo common_hdrs not found.
goto :err

:mklink_failed
cd "..\..\.."
echo mklink failed. You need to run prep-build.bat as Administrator.
goto :err


:err
exit /b %errorlevel%
