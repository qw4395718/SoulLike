// Public/Class/SL_PlayerStateBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SL_PlayerStateBase.generated.h"

/**
 * 基础玩家状态
 * 存放玩家运行时的个人数据（职业、等级、属性等），通过 Replicated 自动同步
 */
UCLASS()
class SOULLIKEDEMO_API ASL_PlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:
	ASL_PlayerStateBase();

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 获取当前职业ID
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	int32 GetClassID() const { return PlayerClassID; }

	// 客户端请求设置职业ID（走 Server RPC）
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void RequestSetClassID(int32 InNewClassID);

	// Server RPC：客户端请求修改职业ID（服务器做权威更新）
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetClassID(int32 InNewClassID);

protected:
	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 复制回调：ClassID 同步到客户端时触发
	UFUNCTION()
	virtual void OnRep_PlayerClassID();

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 玩家职业ID（复制，同步后触发 OnRep_PlayerClassID）
	UPROPERTY(ReplicatedUsing = OnRep_PlayerClassID, BlueprintReadOnly, Category = "PlayerState")
	int32 PlayerClassID;
};
