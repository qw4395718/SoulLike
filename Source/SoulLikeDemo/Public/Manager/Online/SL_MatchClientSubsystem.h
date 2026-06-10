// Public/Component/Online/SL_MatchClientSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SL_MatchClientSubsystem.generated.h"

// 委托声明（用于接收中间服务的消息）
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchServerConnected, bool /*bSuccess*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSignQueryResult, const FString& /*JSONResult*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSummonRequested, const FString& /*SignID*/, const FString& /*RequesterName*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSummonAccepted, const FString& /*SignID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSummonDeclined, const FString& /*SignID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhantomDataReceived, const FString& /*JSONData*/);

/**
 * 中间匹配服务客户端组件
 * 挂在 GameInstance 上，管理到 Python 匹配服务的 TCP Socket 连接
 */
UCLASS()
class SOULLIKEDEMO_API USL_MatchClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USL_MatchClientSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/

	// 连接到中间匹配服务
	UFUNCTION(BlueprintCallable, Category = "MatchClient")
	bool Connect(const FString& InIP, int32 InPort);

	// 断开连接
	UFUNCTION(BlueprintCallable, Category = "MatchClient")
	void Disconnect();

	// 是否已连接
	UFUNCTION(BlueprintPure, Category = "MatchClient")
	bool IsConnected() const { return bIsConnected; }

	// ===== 消息发送 =====

	// 注册本世界实例
	void RegisterInstance(const FString& InInstanceID, const FString& InMapName,
		const FString& InIP, int32 InPort);

	// 注销本世界实例
	void UnregisterInstance();

	// 注册召唤标记
	void RegisterSign(const FString& InOwnerName, int32 InLevel,
		int32 InWeaponLevel, const FString& InMapName,
		const FString& InTransformJSON, float InTimeRemaining);

	// 移除召唤标记
	void UnregisterSign(const FString& InSignID);

	// 查询可用标记
	void QuerySigns(const FString& InMapName, int32 InLevel, int32 InWeaponLevel);

	// 请求召唤
	void RequestSummon(const FString& InSignID, const FString& InRequesterName,
		const FString& InRequesterInstance, int32 InRequesterLevel);

	// 接受召唤
	void AcceptSummon(const FString& InSignID, const FString& InRequesterInstance);

	// 拒绝召唤
	void DeclineSummon(const FString& InSignID, const FString& InRequesterInstance);

	// 传输灵体数据
	void TransferPhantomData(const FString& InTargetInstance, const FString& InDataJSON);

	// ===== 委托（外部绑定回调） =====

	FOnMatchServerConnected OnConnected;
	FOnSignQueryResult OnSignQueryResult;
	FOnSummonRequested OnSummonRequested;
	FOnSummonAccepted OnSummonAccepted;
	FOnSummonDeclined OnSummonDeclined;
	FOnPhantomDataReceived OnPhantomDataReceived;

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/

	// 发送原始 JSON 字符串
	void SendMessage(const FString& InJSON);

	// Tick轮询：接收并解析数据
	bool Tick(float DeltaTime);

	// 处理收到的完整消息行
	void ProcessMessage(const FString& InLine);

	// 清理 Socket
	void CleanUpSocket();

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/

	UPROPERTY()
	FString ServerIP;

	UPROPERTY()
	int32 ServerPort;

	FString InstanceID;

	class FSocket* MatchSocket;
	class ISocketSubsystem* SocketSubsystem;
	FString ReceiveBuffer;
	FDelegateHandle TickHandle;

	bool bIsConnected;
};
