// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_EquipmentBar.generated.h"

class UHUD_ItemIcon;

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
	
	UFUNCTION(BlueprintCallable, Category = "EquipmentBarBar")
		void InitializeEquipmentBar(
			UHUD_ItemIcon* UpEquipment,
			UHUD_ItemIcon* DownEquipment,
			UHUD_ItemIcon* LeftEquipment,
			UHUD_ItemIcon* RightEquipment,
			UHUD_ItemIcon* UpSecondEquipment,
			UHUD_ItemIcon* UpThirdEquipment,
			UHUD_ItemIcon* UpFourthEquipment,
			UHUD_ItemIcon* DownSecondEquipment,
			UHUD_ItemIcon* DownThirdEquipment,
			UHUD_ItemIcon* DownFourthEquipment
			);

	UFUNCTION(BlueprintCallable, Category = "EquipmentBarBar")
		void UpdateTargetSlot();


protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用
	UPROPERTY()
		TMap<EHUDEquipmentSlotType, UHUD_ItemIcon*> EquipmentSotMap;

};

