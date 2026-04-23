// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Engine/StreamableManager.h"
#include "UObject/NameTypes.h"
#include <GameplayTagContainer.h>
#include "SL_ComboManagerComponent.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class EComboInputActionType :uint8
{
	EComboInputAction_Min			UMETA(DisplayName = "None"),
	EComboInputAction_Light			UMETA(DisplayName = "Light"),
	EComboInputAction_Height		UMETA(DisplayName = "Height"),
	EComboInputAction_Special		UMETA(DisplayName = "Special"),
	EComboInputAction_Max			UMETA(DisplayName = "Light"),
};


USTRUCT(BlueprintType)
struct FComboInfo
{
	GENERATED_BODY()

	// 当前连击所需要的Tag窗口名
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag	ActiveRequireWindowTag;

	// 连击所绑定的输入按键
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EComboInputActionType	InputActionType;

	// 连击所属GA
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> NextAbilityClass;
};


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

protected:
	/************************************************************************/
	/*                              内部变量                                        */
	/************************************************************************/
	// 连击系统相关数据
	TMap<FGameplayTag, TMap<EComboInputActionType, FComboInfo>> ComboInfoMap;
	
};
