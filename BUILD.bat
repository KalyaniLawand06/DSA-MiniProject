@echo off
setlocal
cd /d "%~dp0"
set SRC=backend\main.cpp backend\Student.cpp backend\Compatibility.cpp backend\Graph.cpp backend\DSU.cpp backend\GreedyGrouping.cpp backend\BFSGrouping.cpp
set OUT=backend\studygroup_server.exe

set GPP=
where g++ >nul 2>&1 && set GPP=g++
if "%GPP%"=="" if exist "tools\w64devkit\bin\g++.exe" set GPP=tools\w64devkit\bin\g++.exe

if not "%GPP%"=="" (
  echo Building with %GPP% ...
  "%GPP%" -std=c++17 -O2 -static -o %OUT% %SRC% -lws2_32
  goto :done
)
where cl >nul 2>&1
if not errorlevel 1 (
  echo Building with Visual C++ ...
  cl /nologo /std:c++17 /EHsc /O2 /Fe:%OUT% %SRC% ws2_32.lib
  goto :done
)
echo.
echo No C++ compiler found. Easiest fix (no installer needed):
echo   1. Download w64devkit ^(a single zip^) from https://github.com/skeeto/w64devkit/releases
echo   2. Extract it so that  tools\w64devkit\bin\g++.exe  exists
echo   3. Run BUILD.bat again
:done
if exist "%OUT%" (echo Build OK: %OUT%) else (echo Build FAILED.)
if /I not "%1"=="nopause" pause
