// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;
class UTextBlock;

// 玩家状态界面组件类型
UENUM(BlueprintType)
enum class EPlayerStatusAttributeType :uint8
{
	EPlayerStatusAttribute_None,
	EPlayerStatusAttribute_Health,
	EPlayerStatusAttribute_Magic,
	EPlayerStatusAttribute_Stamin,
	EPlayerStatusAttribute_Max
};

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_PlayerStatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_PlayerStatusBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 绑定委托
	UFUNCTION()
	void BindGlobalDelegatesEvent();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void SetProgressBarLimit(EPlayerStatusAttributeType AttributeType, float Min, float Max);
		virtual void SetProgressBarLimit_Implementation(EPlayerStatusAttributeType AttributeType, float Min, float Max);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void UpdateProgressInfo(EPlayerStatusAttributeType AttributeType, float OldValue,float CurrentValue);
		virtual void UpdateProgressInfo_Implementation(EPlayerStatusAttributeType AttributeType, float OldValue, float CurrentValue);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void ChangePlayerStatus(EPawnStatusOperation OperationType,TArray<FStatusEffectInfo>& ChangeStatusArr);
		virtual void ChangePlayerStatus_Implementation(EPawnStatusOperation OperationType,const TArray<FStatusEffectInfo>& StatusArr);

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
		UHUD_ProgressBar* m_staminProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_ProgressBar* m_magicProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UHUD_StatusBar* m_playerStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_healthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_staminText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_magicText;

};

