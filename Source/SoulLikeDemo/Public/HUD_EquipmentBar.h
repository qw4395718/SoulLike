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
			UHUD_ItemIcon* upEquipment,
			UHUD_ItemIcon* downEquipment,
			UHUD_ItemIcon* leftEquipment,
			UHUD_ItemIcon* rightEquipment,
			UHUD_ItemIcon* upSecondEquipment,
			UHUD_ItemIcon* upThirdEquipment,
			UHUD_ItemIcon* downSecondEquipment,
			UHUD_ItemIcon* downThirdEquipment
			);

	UFUNCTION(BlueprintCallable, Category = "EquipmentBarBar")
		void UpdateTargetSlot(int type,FStatusEffectInfo status);


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

