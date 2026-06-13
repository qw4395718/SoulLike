@echo off

set ENGINE_DIR=D:\Software\UnrealEngine\UE_4.26

set PROJECT_DIR=%~dp0
set PROJECT_PATH=%PROJECT_DIR%SoulLikeDemo.uproject
set GAME_PORT=17778

call :Launch
goto :EOF

:Launch
if not "%ENGINE_DIR%"=="" (
    "%ENGINE_DIR%\Engine\Binaries\Win64\UE4Editor.exe" "%PROJECT_PATH%" -game -GamePort=%GAME_PORT% -MatchServerIP=127.0.0.1 -MatchServerPort=7777
    goto :EOF
)

where UE4Editor.exe >nul 2>nul
if %ERRORLEVEL% equ 0 (
    UE4Editor.exe "%PROJECT_PATH%" -game -GamePort=%GAME_PORT% -MatchServerIP=127.0.0.1 -MatchServerPort=7777
    goto :EOF
)

echo UE4Editor.exe not found. Edit this script and set ENGINE_DIR, e.g.:
echo   set ENGINE_DIR=D:\Software\UnrealEngine\UE_4.26
pause
