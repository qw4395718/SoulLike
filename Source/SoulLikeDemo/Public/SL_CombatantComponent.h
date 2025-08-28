// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Combat_IF.h"
#include "SL_CombatantComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_CombatantComponent : public UActorComponent ,public ICombat_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_CombatantComponent();

public:
	/************************************************************************/
	/*                              接口实现                                        */
	/************************************************************************/
	UFUNCTION(BlueprintCallable)
		void TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		void PerformExecuted(FName WeaponName)override;

	UFUNCTION(BlueprintCallable)
		void PerformBackStabbed() override;
};
