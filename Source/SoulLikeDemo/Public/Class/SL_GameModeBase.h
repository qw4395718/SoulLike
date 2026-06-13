// Public/Game/SL_GameModeBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SL_GameModeBase.generated.h"

class ALevelManager;
class USL_GameSaveSubsystem;
class ASL_PhantomCharacter;

UCLASS()
class SOULLIKEDEMO_API ASL_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASL_GameModeBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 跨世界灵体追踪：覆盖 PreLogin 捕获 PhantomSession 参数
	virtual void PreLogin(const FString& Options, const FString& Address,
		const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// 跨世界灵体追踪：PostLogin 时将 PhantomCharacter Possess 给对应客户端
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// ===== 关卡控制（供外部调用） =====
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void StartTargetLevel(int32 InLevelID);

	// 设置是否使用存档 
    UFUNCTION(BlueprintCallable, Category = "GameMode")
   		void SetUseSaveData(bool bInUseSaveData) { bUseSaveData = bInUseSaveData; }

	// 获取是否使用存档 
    UFUNCTION(BlueprintPure, Category = "GameMode")
    	bool GetUseSaveData() const { return bUseSaveData; }

    // 检查是否存在存档 
    UFUNCTION(BlueprintPure, Category = "GameMode")
  	  	bool HasSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void LoadLevel(int32 InLevelID);

	// ===== 存档控制（供外部调用） =====
	// 保存当前游戏进度
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void SaveCurrentProgress();

	// ===== 获取引用 =====
	UFUNCTION(BlueprintPure, Category = "GameMode")
		ALevelManager* GetLevelManager() const { return LevelManager; }

	// ===== 跨世界灵体追踪（供 SummonSessionComponent 调用） =====
	// 注册一个 PhantomCharacter，等待灵体客户端连接后 Possess
	void RegisterPendingPhantom(const FString& InSessionID, ASL_PhantomCharacter* InPhantom);
	// 尝试取出匹配的 PhantomCharacter
	ASL_PhantomCharacter* TakePendingPhantom(const FString& InSessionID);

protected:
	/** 创建 LevelManager */
	void CreateLevelManager();

	/** 初始化玩家 */
	void InitializePlayer(int32 InPlayerClassID);

	// 重试 Phantom Possession（客户端先到、Phantom 后生成时使用）
	void RetryPhantomPossession(const FString& InSessionID);

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	UPROPERTY()
		ALevelManager* LevelManager;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		TSubclassOf<ALevelManager> LevelManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Config")
		float LevelStartDelay = 2.0f;

	// 是否使用存档数据（由主菜单设置）
	bool bUseSaveData;

	// ===== 跨世界灵体追踪（仅服务器） =====
	// 从连接 URL 中提取到的等待灵体的 SessionID 列表
	TArray<FString> ExpectedPhantomSessions;
	// 已生成但还未被领取的 PhantomCharacter（SessionID → Phantom）
	TMap<FString, ASL_PhantomCharacter*> PendingPhantoms;
	// 客户端已到达但 PhantomCharacter 尚未生成的等待队列
	TMap<FString, APlayerController*> WaitingPhantomControllers;
};
