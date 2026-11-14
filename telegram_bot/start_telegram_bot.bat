@echo off
title OpenAksara - Telegram Bot Worker
color 0A
echo =================================================================
echo        OpenAksara - Standalone Telegram Bridge Worker
echo =================================================================
echo Menghubungkan ke Cloud Firestore dan memantau data baru...
echo.

cd /d "%~dp0"
node telegram_bot.js
pause
