// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Engine/StreamableManager.h"
#include "UObject/NameTypes.h"
#include <GameplayTagContainer.h>
#include "SL_ComboManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_ComboManagerComponent : public UActorComponent 
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_ComboManagerComponent();

public:
	/************************************************************************/
	/*                              接口实现                                        */
	/************************************************************************/


public:
	/************************************************************************/
	/*                              外部调用                                        */
	/************************************************************************/
	// 委托-响应输入
	UFUNCTION()
		void HandleInputPressed(EComboInputActionType InputType);

protected:
	/************************************************************************/
	/*                              内部调用                                        */
	/************************************************************************/
	/** 发送GameplayEvent给ASC */
	void SendComboEvent(const FGameplayTag& EventTag, TSubclassOf<UGameplayAbility> NextAbility);

protected:
	/************************************************************************/
	/*                              内部变量                                        */
	/************************************************************************/
	
};
