// Public/Save/USL_GameSaveSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "USL_GameSaveData.h"
#include "USL_GameSaveSubsystem.generated.h"

/**
 * 游戏存档管理子系统
 * 通过 UGameInstanceSubsystem 生命周期管理
 */
UCLASS()
class SOULLIKEDEMO_API USL_GameSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USL_GameSaveSubsystem();

	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	virtual void Initialize(FSubsystemCollectionBase& InCollection) override;
	virtual void Deinitialize() override;

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 保存游戏进度
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
		bool SaveGame(int32 InLevelID, int32 InPlayerClassID);

	// 加载游戏进度
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
		bool LoadGame(int32& OutLevelID, int32& OutPlayerClassID);

	// 检查是否存在存档
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
		bool HasSaveData() const;

	// 删除存档
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
		bool DeleteSaveData();

	// 获取存档对象（供蓝图读取详细信息）
	UFUNCTION(BlueprintPure, Category = "SaveSystem")
		USL_GameSaveData* GetSaveData() const { return CurrentSaveData; }

	// 获取全局单例
	UFUNCTION(BlueprintPure, Category = "SaveSystem", meta = (WorldContext = "WorldContextObject"))
		static USL_GameSaveSubsystem* Get(const UObject* WorldContextObject);

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 当前存档名称
	static const FString SaveSlotName;

	// 存档的用户索引（默认0）
	static const int32 SaveUserIndex;

	// 当前存档数据
	UPROPERTY()
		USL_GameSaveData* CurrentSaveData;
};