// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponMeleeAttackComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UWeaponMeleeAttackComponent::UWeaponMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponMeleeAttackComponent::InitalizeWeaponComponent(AWeaponBase* Onwer, FVector CBSize)
{

}

void UWeaponMeleeAttackComponent::EnableCollisionBoxCheck()
{

}

void UWeaponMeleeAttackComponent::DisableCollisionBoxCheck()
{

}

void UWeaponMeleeAttackComponent::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void UWeaponMeleeAttackComponent::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void UWeaponMeleeAttackComponent::ApplyDamageToOverlappingActors()
{

}

void UWeaponMeleeAttackComponent::EnableParryWindowCheck()
{

}

void UWeaponMeleeAttackComponent::DisableParryWindowCheck()
{

}

bool UWeaponMeleeAttackComponent::IsActiveParryWindow()
{
	return bActiveParryWindow;
}
