// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/ActorComponent.h"
#include "CharacterComponent_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UCharacterComponent_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API ICharacterComponent_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION()
		virtual UActorComponent* GetCombatantComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetEquipmentComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetHealthComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetInventoryComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetSpecialMovementComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetStaminaComponent() = 0;

	UFUNCTION()
		virtual UActorComponent* GetStateComponent() = 0;
};
