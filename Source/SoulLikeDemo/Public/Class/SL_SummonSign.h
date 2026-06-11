#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Struct/OnlineSummonStructs.h"
#include "SL_SummonSign.generated.h"

class UWidgetComponent;

/**
 * 召唤标记 Actor
 * 在放置者的世界中生成，其他玩家可见并可交互
 */
UCLASS()
class SOULLIKEDEMO_API ASL_SummonSign : public AActor
{
	GENERATED_BODY()

public:
	ASL_SummonSign();

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 初始化标记数据
	UFUNCTION(BlueprintCallable, Category = "SummonSign")
	void InitializeSign(const FSummonSignInfo& InSignInfo);

	// 获取标记数据
	UFUNCTION(BlueprintPure, Category = "SummonSign")
	const FSummonSignInfo& GetSignInfo() const { return SignInfo; }

	// 获取标记唯一ID
	UFUNCTION(BlueprintPure, Category = "SummonSign")
	FGuid GetSignID() const { return SignInfo.SignID; }

	// 锁定标记（被召唤中）
	UFUNCTION(BlueprintCallable, Category = "SummonSign")
	void LockSign();

	// 取消标记（移除）
	UFUNCTION(BlueprintCallable, Category = "SummonSign")
	void RemoveSign();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 交互重叠检测
	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 标记过期处理
	void OnSignExpired();

	// 注册到 SignManager
	void RegisterToManager();

	// 从 SignManager 注销
	void UnregisterFromManager();

protected:
	/************************************************************************/
	/*                               组件                                   */
	/************************************************************************/
	// 场景根组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	// 交互碰撞体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractionCollision;

	// 可视化网格体（美术资源，可在蓝图中替换）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VisualMesh;

	// 标记头顶显示名称（可选）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* NameWidget;

	/************************************************************************/
	/*                               数据                                   */
	/************************************************************************/
	// 标记数据
	UPROPERTY(ReplicatedUsing = OnRep_SignInfo)
	FSummonSignInfo SignInfo;

	UFUNCTION()
	void OnRep_SignInfo();

public:
	// 是否已被锁定
	UPROPERTY(Replicated)
	bool bIsLocked;

	// Phase 2: 是否为远程实例的标记
	UPROPERTY()
	bool bIsRemoteSign;

	// Phase 2: 中间服务中的远程 sign_id
	UPROPERTY()
	FString RemoteSignID;

	// Phase 2: 所属远程实例 ID
	UPROPERTY()
	FString RemoteInstanceID;

	// 外部访问
	// 获取当前重叠的角色列表
	const TArray<TWeakObjectPtr<AActor>>& GetOverlappingPlayers() const { return OverlappingPlayers; }
	// 检查某个角色是否正在重叠此标记
	bool IsOverlappedBy(const AActor* InActor) const;

protected:
	// 内部访问 — 当前正在重叠此标记的角色列表（仅服务器端）
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> OverlappingPlayers;

};