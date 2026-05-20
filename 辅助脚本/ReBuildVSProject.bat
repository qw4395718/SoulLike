@echo off
set ENGINE_PATH=C:\Program Files\Epic Games\UE_4.26
set PROJECT_PATH=D:\Code\SoulLike\SoulLikeDemo.uproject
set OUTPUTLOG_PATH=D:\Code\SoulLike\Saved\Logs

:: BuildToolPath
set UE_BUILD_TOOL_PATH=%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool.exe

:: RebuildProject
"%ENGINE_PATH%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="%PROJECT_PATH%" -game > "%OUTPUTLOG_PATH%\ReBuildVSProject_Output.log" 2>&1

pause