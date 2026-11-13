@echo off
title OpenAksara Waypoint Auto-Updater
cd /d "%~dp0"
powershell -ExecutionPolicy Bypass -NoProfile -File "%~dp0update_waypoint.ps1"

