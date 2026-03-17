// GlobalDelegatesManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalDelegatesManager.generated.h"

// 前向声明
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnClickInterActBtnToDialog, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClickInterMainMenuButton, const int);

UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UGlobalDelegatesManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "Global Delegates", meta = (WorldContext = "WorldContextObject"))
		static UGlobalDelegatesManager* Get(const UObject* WorldContextObject);

	/************************************************************************/
	/*                              委托实例                                        */
	/************************************************************************/
	// 交互按钮行为委托
		FOnClickInterActBtnToDialog OnClickInterActBtnToDialog;
	// 菜单按钮行为委托
		FOnClickInterMainMenuButton OnClickInterMainMenuButton;
	// 血量变动响应委托
		

	/**
	 * 委托响应函数
	 */
	// 交互按钮行为响应
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastDialogShow(const FString& labelText, const FString& descText);

	// 菜单按钮行为响应
	UFUNCTION(BlueprintCallable, Category = "Global Delegates")
		void BroadcastOnClickMainMenuButton(const int widgetIndex);


protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 解除所有委托绑定
	void ClearAllBinding();
};