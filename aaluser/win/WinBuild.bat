call "%VCINSTALLDIR%\vcvarsall.bat" x86
msbuild.exe .\WINAALSDK.sln /p:Configuration="Win8.1 Debug" /p:Platform=x64
