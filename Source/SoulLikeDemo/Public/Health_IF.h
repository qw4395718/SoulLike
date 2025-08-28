// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Health_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UHealth_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API IHealth_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable)
		virtual bool IsAlive() = 0;

	UFUNCTION(BlueprintCallable)
		virtual float GetCurrentHealthValue() = 0;

	UFUNCTION(BlueprintCallable)
		virtual float GetMaxHealthValue() = 0;

	UFUNCTION(BlueprintCallable)
		virtual void ReduceCurrentHealth(float ReduceValue) = 0;

	UFUNCTION(BlueprintCallable)
		virtual void ReviveCurrentHealth(float ReviveValue) = 0;

};
