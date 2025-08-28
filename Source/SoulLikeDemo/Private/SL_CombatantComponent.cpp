// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CombatantComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_CombatantComponent::USL_CombatantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_CombatantComponent::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

}

