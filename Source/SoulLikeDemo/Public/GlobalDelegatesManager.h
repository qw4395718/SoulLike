// GlobalDelegatesManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalDelegatesManager.generated.h"

// 前向声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClickInterActBtnToDialog, FString, labelText, FString, descText);

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UGlobalDelegatesManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "Global Delegates", meta = (WorldContext = "WorldContextObject"))
		static UGlobalDelegatesManager* Get(const UObject* WorldContextObject);

	// 委托实例
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
		FOnClickInterActBtnToDialog OnClickInterActBtnToDialog;

	// 工具函数：绑定原始函数指针
	template<typename FuncPtr>
	FDelegateHandle BindDialogShow(FuncPtr InFuncPtr)
	{
		return OnClickInterActBtnToDialog.AddStatic(InFuncPtr);
	}

	// 触发事件的函数
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastDialogShow(const FString& labelText, const FString& descText);

private:
	// 私有构造函数
	UGlobalDelegatesManager();

	// 存储单例实例
	static UGlobalDelegatesManager* Instance;
};