@echo off
setlocal
cd /d "%~dp0"
title Smart Study Group Formation System

if not exist "backend\studygroup_server.exe" (
  echo Compiled backend not found - trying to build it from source...
  call BUILD.bat nopause
)
if not exist "backend\studygroup_server.exe" (
  echo.
  echo ERROR: backend\studygroup_server.exe does not exist and could not be built.
  echo See README.txt - section "Getting the .exe".
  pause
  exit /b 1
)

echo Starting C++ backend on http://localhost:8080 ...
start "StudyGroup Server (close to stop)" cmd /c "backend\studygroup_server.exe || (echo. & echo Server stopped with an error. & pause)"
timeout /t 2 /nobreak >nul

tasklist /FI "IMAGENAME eq studygroup_server.exe" 2>nul | find /I "studygroup_server.exe" >nul
if errorlevel 1 (
  echo.
  echo ERROR: The server could not start. Port 8080 may be in use - run STOP.bat and try again.
  pause
  exit /b 1
)
start "" "http://localhost:8080"
echo Application opened in your browser. Use STOP.bat to shut the server down.
timeout /t 3 >nul
