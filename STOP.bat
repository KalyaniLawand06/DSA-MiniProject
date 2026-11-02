@echo off
cd /d "%~dp0"
echo Stopping Study Group server...
curl -s -X POST http://localhost:8080/api/shutdown >nul 2>&1
timeout /t 1 /nobreak >nul
taskkill /IM studygroup_server.exe /F >nul 2>&1
echo Done.
timeout /t 2 >nul
