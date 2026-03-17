// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_BossStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;
class UTextBlock;

// 玩家状态界面组件类型
UENUM(BlueprintType)
enum class EBossStatusAttributeType :uint8
{
	EBossStatusAttribute_None,
	EBossStatusAttribute_Health,
	EBossStatusAttribute_Magic,
	EBossStatusAttribute_Stamin,
	EBossStatusAttribute_Max
};

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_BossStatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_BossStatusBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,Category = "Boss StatusBar")
		void SetBossProgressBarLimit(EBossStatusAttributeType AttributeType, float Min, float Max);
		virtual void SetBossProgressBarLimit_Implementation(EBossStatusAttributeType AttributeType, float Min, float Max);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,Category = "Player StatusBar")
		void UpdateBossProgressInfo(EBossStatusAttributeType AttributeType, float CurrentValue);
		virtual void UpdateBossProgressInfo_Implementation(EBossStatusAttributeType AttributeType, float CurrentValue);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void ChangeBossStatus(EPawnStatusOperation OperationType, TArray<FStatusEffectInfo>& ChangeStatusArr);
		virtual void ChangeBossStatus_Implementation(EPawnStatusOperation OperationType, const TArray<FStatusEffectInfo>& ChangeStatusArr);


	UFUNCTION(BlueprintCallable, Category = "Boss StatusBar")
		void SetBossName(FString bossName);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_ProgressBar* m_healthProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_StatusBar* m_bossStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_bossNameText;

};

