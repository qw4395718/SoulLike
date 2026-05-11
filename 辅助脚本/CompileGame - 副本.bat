@echo off
set UBT_PATH="C:\Program Files\Epic Games\UE_4.26\Engine\Binaries\DotNET\UnrealBuildTool.exe"
set PROJECT_PATH="D:\Code\SoulLike\SoulLikeDemo.uproject"

echo Compile SoulLikeDemo (Development)...
%UBT_PATH% SoulLikeDemo Win64 Development -Project=%PROJECT_PATH% -Progress

pause