// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_PlayerStatusBar.h"
#include <ScreenWidget_IF.h>
#include "HUD_PawnStatusBarInScreen.generated.h"

class UHUD_ProgressBar;
class UHUD_StatusBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_PawnStatusBarInScreen : public UUserWidget,public IScreenWidget_IF
{
	GENERATED_BODY()
public:
	UHUD_PawnStatusBarInScreen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 绑定委托
	UFUNCTION()
	void BindGlobalDelegatesEvent();

	UFUNCTION()
	void UnbindGlobalDelegatesEvent();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void SetProgressBarLimit(EPlayerStatusAttributeType AttributeType, float Min, float Max);
		virtual void SetProgressBarLimit_Implementation(EPlayerStatusAttributeType AttributeType, float Min, float Max);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void UpdateProgressInfo(EPlayerStatusAttributeType AttributeType, float OldValue,float CurrentValue);
		virtual void UpdateProgressInfo_Implementation(EPlayerStatusAttributeType AttributeType, float OldValue, float CurrentValue);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player StatusBar")
		void ChangePlayerStatus(EPawnStatusOperation OperationType,TArray<FStatusEffectInfo>& ChangeStatusArr);
		virtual void ChangePlayerStatus_Implementation(EPawnStatusOperation OperationType,const TArray<FStatusEffectInfo>& StatusArr);

	/************************************************************************/
	/*                                    接口实现                                  */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Player StatusBar")
	void SetOwningPawn(AActor* OwnPawn) override;

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
	UHUD_StatusBar* m_playerStatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* m_healthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock *Text_HealthPG;

	AActor* OwningPawn;
};

