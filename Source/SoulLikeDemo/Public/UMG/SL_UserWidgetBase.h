// SL_UserWidgetBase.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Manager/UI/SL_UILayerDefine.h"
#include "Manager/UI/SL_UINavigationTypes.h"
#include "SL_UserWidgetBase.generated.h"

/******/
/* 页面Widget基类
 * 所有参与层级/导航/自动InputMode管理的Widget建议继承此类
 * 蓝图仍可继承，基类提供C++虚钩子
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API USL_UserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/************************************************************************/
	/* 导航生命周期                                                                   */
	/************************************************************************/
	// 页面被导航到（打开 / UIManager创建后触发）
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Navigation")
		void OnNavigatedTo(const FUINavigationContext& InContext);
	virtual void OnNavigatedTo_Implementation(const FUINavigationContext& InContext);

	// 页面被导航离开（关闭 / 隐藏时触发）
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Navigation")
		void OnNavigatedFrom(const FUINavigationContext& InContext);
	virtual void OnNavigatedFrom_Implementation(const FUINavigationContext& InContext);

	// 页面从返回栈中恢复时触发（NavigateBack后新栈顶收到此事件）
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Navigation")
		void OnNavigatedBack(const FUINavigationContext& InContext);
	virtual void OnNavigatedBack_Implementation(const FUINavigationContext& InContext);

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// Widget类型（UIManager在创建时自动设置）
	UFUNCTION(BlueprintPure, Category = "UI")
		EWidgetType GetWidgetType() const { return WidgetType; }
	UFUNCTION(BlueprintCallable, Category = "UI")
		void SetWidgetType(EWidgetType InType) { WidgetType = InType; }

	// UI层级
	UFUNCTION(BlueprintPure, Category = "UI|Layer")
		EUILayer GetUILayer() const { return Layer; }
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
		void SetUILayer(EUILayer InLayer) { Layer = InLayer; }

	// InputMode需求（Inherit时自动从层级配置解析）
	UFUNCTION(BlueprintPure, Category = "UI|InputMode")
		EUIInputModeRequirement GetInputModeRequirement() const { return InputModeRequirement; }
	UFUNCTION(BlueprintCallable, Category = "UI|InputMode")
		void SetInputModeRequirement(EUIInputModeRequirement InReq) { InputModeRequirement = InReq; }

	// 获取实际InputMode需求（Inherit会被解析为层级默认值）
	UFUNCTION(BlueprintPure, Category = "UI|InputMode")
		EUIInputModeRequirement GetResolvedInputModeRequirement() const;

	// 是否参与导航栈管理
	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
		bool IsNavigationPage() const { return bIsNavigationPage; }
	UFUNCTION(BlueprintCallable, Category = "UI|Navigation")
		void SetIsNavigationPage(bool bInValue) { bIsNavigationPage = bInValue; }

	// 通过UIManager关闭自身
	UFUNCTION(BlueprintCallable, Category = "UI")
		virtual void CloseSelf();

protected:
	/************************************************************************/
	/* 内部变量 - Blueprint编辑默认值                                                  */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, Category = "UI|Layer")
		EUILayer Layer = EUILayer::HUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI|InputMode")
		EUIInputModeRequirement InputModeRequirement = EUIInputModeRequirement::Inherit;

	// 默认参与导航栈（Popup及以上层级自动入栈）
	UPROPERTY(EditDefaultsOnly, Category = "UI|Navigation")
		bool bIsNavigationPage = true;

private:
	// UIManager在创建Widget时自动设置的枚举类型
	UPROPERTY()
		EWidgetType WidgetType = EWidgetType::EWIDGET_None;
};
