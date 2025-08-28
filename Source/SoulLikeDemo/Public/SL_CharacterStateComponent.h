// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "CharacterInfo_IF.h"
#include "SL_CharacterStateComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_CharacterStateComponent : public UActorComponent, public ICharacterInfo_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_CharacterStateComponent();

public:
/************************************************************************/
/*                            接口实现                                          */
/************************************************************************/

	UFUNCTION(BlueprintCallable)
		bool CanExecute();

	UFUNCTION(BlueprintCallable)
		bool CanBackStabs();

public:
/************************************************************************/
/*                              外部调用                                        */
/************************************************************************/

	UFUNCTION(BlueprintCallable)
		void InitCharacterStateInfo();

	UFUNCTION(BlueprintCallable)
		void SetIsReadyForExecution(bool bReady);



protected:
/************************************************************************/
/*                             内部变量                                        */
/************************************************************************/
	// 角色状态-是否处于待处决状态
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bIsReadyForExecution;

	// 角色状态-是否处于待处决状态
	UPROPERTY(EditDefaultsOnly, Category = "CharacterState")
		bool bCanBackStab;
};
