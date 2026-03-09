// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_EquipmentBar.generated.h"

class UHUD_ItemIcon;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_EquipmentBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_EquipmentBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 初始化组件
	UFUNCTION(BlueprintCallable, Category = "EquipmentBarBar")
		void InitializeEquipmentBar();
	// 更新指定槽位数据
	UFUNCTION(BlueprintCallable, Category = "EquipmentBarBar")
		void UpdateTargetSlot(EHUDEquipmentSlotType type,FStatusEffectInfo status);
	// 开始切换装备UI动画
	UFUNCTION(BlueprintCallable)
		void StartChangeEquipment(UWidgetAnimation* Anim);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
		// 重写原生鼠标滚轮事件
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 重写原生Tick事件
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

	// 鼠标滚动行为处理
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category = "Scroll")
	void HandleScroll(float wheelDelta);
	virtual void HandleScroll_Implementation(float wheelDelta);

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用
	UPROPERTY(meta = (BindWidget))
		UHUD_ItemIcon* m_upSlot;

	UPROPERTY(meta = (BindWidget))
		UHUD_ItemIcon* m_downSlot;

	UPROPERTY(meta = (BindWidget))
		UHUD_ItemIcon* m_leftSlot;

	UPROPERTY(meta = (BindWidget))
		UHUD_ItemIcon* m_rightSlot;

	UPROPERTY()
		TMap<EHUDEquipmentSlotType, UHUD_ItemIcon*> EquipmentSotMap;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* m_changeEquipmentAnimation;

	bool m_bIsCtrlHeldInternal = false;
private:
	/************************************************************************/
	/* 内部变量-配置                                                                     */
	/************************************************************************/
	// 滚动速度控制
	UPROPERTY(EditAnywhere, Category = "Scroll")
		float scrollSensitivity = 40.0f;

	// 滚动速度控制
	UPROPERTY(EditAnywhere, Category = "Scroll")
		float scrollHeightLimit = 40.0f;
};

