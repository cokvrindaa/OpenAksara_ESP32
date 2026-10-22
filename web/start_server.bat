@echo off
title OpenAksara EduCloud - IoT Monitoring Server
color 0B
echo =================================================================
echo        OpenAksara EduCloud - IoT Monitoring Server
echo =================================================================
echo Memulai server Node.js pada port 3000...
echo.

cd /d "%~dp0"
start http://localhost:3000
node server.js
pause

