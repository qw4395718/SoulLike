@echo off

echo ============================================
echo   SoulLike Match Server
echo   Listening on 127.0.0.1:7777
echo ============================================

cd /d "%~dp0"
python match_server.py

pause
