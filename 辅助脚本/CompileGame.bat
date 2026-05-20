@echo off
set UBT_PATH="C:\Program Files\Epic Games\UE_4.26\Engine\Binaries\DotNET\UnrealBuildTool.exe"
set PROJECT_PATH="D:\Code\SoulLike\SoulLikeDemo.uproject"
set OUTPUTLOG_PATH=D:\Code\SoulLike\Saved\Logs

echo Compile SoulLikeDemo (Development)...
%UBT_PATH% SoulLikeDemo Win64 Development -Project=%PROJECT_PATH% -Progress > "%OUTPUTLOG_PATH%\CompileGame.log" 2>&1

pause