// GlobalDelegatesManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalDelegatesManager.generated.h"

// 前向声明
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnClickInterActBtnToDialog, const FString&, const FString&);

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UGlobalDelegatesManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "Global Delegates", meta = (WorldContext = "WorldContextObject"))
		static UGlobalDelegatesManager* Get(const UObject* WorldContextObject);

	// 委托实例
		FOnClickInterActBtnToDialog OnClickInterActBtnToDialog;

	// 触发事件的函数
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastDialogShow(const FString& labelText, const FString& descText);

private:
	// 私有构造函数
	UGlobalDelegatesManager();

	// 存储单例实例
	static UGlobalDelegatesManager* Instance;
};