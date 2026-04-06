@echo off
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
call "%VS_PATH%" x64

if not exist compilado\bin mkdir compilado\bin
if not exist compilado\obj mkdir compilado\obj

set "INC_PATH1=%~dp0compilado"
set "INC_PATH2=C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt"
set "INC_PATH3=C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt"

set "LIBS=WindowsApp.lib Shlwapi.lib compilado\lib\ScriptHookV.lib User32.lib"

cl.exe /O2 /MT /EHsc /std:c++17 /LD main.cpp ControllerManager.cpp Telemetry.cpp Config.cpp Menu.cpp /Fo"compilado\obj\\" /Fe"compilado\bin\GTAVImpulseTriggers.asi" /DEF:exports.def /I"%INC_PATH1%" /I"%INC_PATH2%" /I"%INC_PATH3%" %LIBS%
