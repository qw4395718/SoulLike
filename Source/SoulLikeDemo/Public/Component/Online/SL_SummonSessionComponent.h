#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/OnlineSummonStructs.h"
#include "SL_SummonSessionComponent.generated.h"

class ASL_SummonSign;
class ASL_PhantomCharacter;

/**
 * 召唤会话组件
 * 挂在 PlayerController 上，管理玩家的联机状态机和 RPC 通信
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_SummonSessionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USL_SummonSessionComponent();

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 获取当前联机状态
	UFUNCTION(BlueprintPure, Category = "SummonSession")
	EOnlinePlayerState GetCurrentState() const { return CurrentState; }

	// 获取当前玩家名称
	UFUNCTION(BlueprintPure, Category = "SummonSession")
	FString GetPlayerDisplayName() const;

	// 获取当前玩家等级（用于匹配）
	UFUNCTION(BlueprintPure, Category = "SummonSession")
	int32 GetPlayerLevel() const;

	// 获取当前玩家武器等级（用于匹配）
	UFUNCTION(BlueprintPure, Category = "SummonSession")
	int32 GetPlayerWeaponLevel() const;

	// ===== 放置者侧 =====
	// 服务器：放置召唤标记
	UFUNCTION(BlueprintCallable, Category = "SummonSession")
	void PlaceSummonSign();

	// 服务器：取消当前标记
	UFUNCTION(BlueprintCallable, Category = "SummonSession")
	void CancelSummonSign();

	// 服务器：确认被召唤（接受召唤请求）
	UFUNCTION(BlueprintCallable, Category = "SummonSession")
	void AcceptSummon();

	// 服务器：拒绝被召唤
	UFUNCTION(BlueprintCallable, Category = "SummonSession")
	void DeclineSummon();

	// ===== 召唤者侧 =====
	// 服务器：与召唤标记交互，发起召唤请求
	UFUNCTION(BlueprintCallable, Category = "SummonSession")
	void InteractWithSign(ASL_SummonSign* InSign);

	// 查询当前关卡可用的召唤标记
	UFUNCTION(BlueprintPure, Category = "SummonSession")
	TArray<FSummonSignInfo> QueryAvailableSigns() const;

	// Phase 2: 远程查询结果回调
	UFUNCTION()
	void OnRemoteQueryResult(const FString& InResultJSON);

	// Phase 2: 收到远程召唤请求（放置者侧，收到他人召唤自己的请求）
	UFUNCTION()
	void OnSummonRequestReceived(const FString& InSignID, const FString& InRequesterName, const FString& InRequesterInstance, const FString& InRequesterIP, int32 InRequesterPort);

	// Phase 2: 召唤被接受（召唤者侧，自己发起的请求被确认）
	UFUNCTION()
	void OnSummonAcceptedByRemote(const FString& InSignID);

	// Phase 2: 召唤被拒绝（召唤者侧，自己发起的请求被拒绝）
	UFUNCTION()
	void OnSummonDeclinedByRemote(const FString& InSignID);

	// Phase 3: 收到 PhantomData，在本地生成灵体
	UFUNCTION()
	void OnPhantomDataReceived(const FString& InJSONData, const FString& InPlacerInstance);

	// Phase 3: 打包当前角色的 PhantomData（用于灵体传输）
	FPhantomData PackPhantomData() const;

	// Phase 3: 收到 phantom_ready 通知，执行 ClientTravel
	UFUNCTION()
	void OnPhantomReadyReceived(const FString& InSessionID);

	// Phase 3: 收到 ready_query，回复 phantom_ready
	UFUNCTION()
	void OnReadyQueryReceived(const FString& InSessionID, const FString& InRequesterInstance);

	// Phase 3: 召唤超时处理
	void OnSummonTimeout();

	// Phase 3: 收到召唤错误通知
	UFUNCTION()
	void OnSummonErrorReceived(const FString& InSessionID, const FString& InErrorReason);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 切换状态
	void SetState(EOnlinePlayerState InNewState);

	// 道具使用回调（通过 GlobalDelegatesManager 监听）
	UFUNCTION()
	void OnItemUsedCallback(AActor* InUserActor, FName InItemID);

	// 获取 SignManager
	class USL_SignManager* GetSignManager() const;

	// 生成标记 Actor
	ASL_SummonSign* SpawnSummonSignActor(const FTransform& InTransform);

	// 销毁当前标记 Actor
	void DestroyCurrentSign();

	// 获取所属的 PlayerController
	class APlayerController* GetOwningPlayerController() const;

	// 获取所属的 Character
	class ACharacter* GetOwningCharacter() const;

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 当前联机状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SummonSession")
	EOnlinePlayerState CurrentState;

	// 当前放置的标记 Actor（放置者侧）
	UPROPERTY()
	ASL_SummonSign* CurrentSummonSign;

	// 当前正在交互的标记 Actor（召唤者侧）
	UPROPERTY()
	ASL_SummonSign* TargetSummonSign;

	// 当前被召唤的 SignID（本地标记）
	FGuid PendingSummonSignID;

	// Phase 2: 远程待处理召唤请求信息
	FString PendingRemoteSignID;
	FString PendingRequesterName;
	FString PendingRequesterInstance;

	// Phase 3: 召唤者服务器的连接信息（用于 ClientTravel）
	FString PendingRequesterIP;
	int32 PendingRequesterPort;

	// Phase 3: 当前 Phantom 会话 ID（用于 Travel URL 和时序保护）
	FString CurrentPhantomSessionID;

	// Phase 3: 放入者的 InstanceID（B 侧使用，用于发送 phantom_ready）
	FString PlacerInstanceID;

	// Phase 3: ready_query 重试计数器
	int32 ReadyQueryRetryCount;

	// Phase 3: ready_query 定时器
	FTimerHandle ReadyQueryTimerHandle;

	// Phase 2: 远程标记 Actor 追踪（cleanup 用）
	UPROPERTY()
	TMap<FString, ASL_SummonSign*> RemoteSignActors;

	// Phase 2: 生成远程标记 Actor
	void SpawnRemoteSignActor(const FString& InRemoteSignID, const FString& InOwnerName,
		int32 InLevel, const FString& InTransformJSON, const FString& InInstanceID);

	// Phase 2: 清理所有远程标记 Actor
	void ClearRemoteSignActors();

	// 匹配配置
	UPROPERTY(EditDefaultsOnly, Category = "SummonSession|Config")
	FSummonMatchConfig MatchConfig;

	// 标记 Actor 类（可在蓝图或 C++ 中指定子类）
	UPROPERTY(EditDefaultsOnly, Category = "SummonSession|Config")
	TSubclassOf<ASL_SummonSign> SummonSignClass;

	// Phase 3: 灵体角色类（可在蓝图中指定子类，调整骨骼网格体位置/旋转）
	UPROPERTY(EditDefaultsOnly, Category = "SummonSession|Config")
	TSubclassOf<ASL_PhantomCharacter> PhantomCharacterClass;

	// 召唤符道具 ID（对应道具数据表中的 ItemID）
	UPROPERTY(EditDefaultsOnly, Category = "SummonSession|Config")
	FName SummonItemID = FName("SummonSign");
};
