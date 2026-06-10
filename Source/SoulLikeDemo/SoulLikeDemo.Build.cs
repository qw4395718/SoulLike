// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;


public class SoulLikeDemo : ModuleRules
{
	public SoulLikeDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 递归获取所有 Public 子目录
        string publicPath = Path.Combine(ModuleDirectory, "Public");
        if (Directory.Exists(publicPath))
        {
            PublicIncludePaths.AddRange(GetAllDirectories(publicPath));
        }

        // 递归获取所有 Private 子目录
        string privatePath = Path.Combine(ModuleDirectory, "Private");
        if (Directory.Exists(privatePath))
        {
            PrivateIncludePaths.AddRange(GetAllDirectories(privatePath));
        }

        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"UnLua",
            "GameplayAbilities", "GameplayTags", "GameplayTasks",
            "AIModule",
            "Sockets",
            "Networking",
            "Json",
            "JsonUtilities",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Lua" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}

    // 递归获取所有子目录的辅助函数
    private List<string> GetAllDirectories(string basePath)
    {
        var directories = new List<string>();

        // 添加当前目录
        directories.Add(basePath);

        try
        {
            // 递归获取所有子目录
            foreach (string subDir in Directory.GetDirectories(basePath, "*", SearchOption.AllDirectories))
            {
                directories.Add(subDir);
            }
        }
        catch (System.Exception ex)
        {
            // 如果出现访问错误，输出警告
            System.Console.WriteLine("Warning: Failed to get subdirectories for " + basePath + ": " + ex.Message);
        }

        return directories;
    }
}
