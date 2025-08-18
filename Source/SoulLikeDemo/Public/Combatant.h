// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combatant.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCombatant : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API ICombatant
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
		virtual void TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		virtual float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		virtual bool IsAlive() const;
};
