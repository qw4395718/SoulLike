@echo off

echo ============================================
echo   SoulLike Match Server
echo   Listening on 127.0.0.1:7777
echo   Press Ctrl+C to stop
echo ============================================

cd /d "%~dp0MatchServer"
python match_server.py

echo.
echo Server stopped.
pause
