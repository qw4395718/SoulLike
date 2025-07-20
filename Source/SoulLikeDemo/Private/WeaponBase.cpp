// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

void UWeaponBase::InitializeWeapon(ASoulLikeCharacter* OwnerCharacter)
{

}

FDamageData UWeaponBase::GetDamageData_Implementation() const
{
	return FDamageData();
}

void UWeaponBase::PlayAttackMontage_Implementation(EAttackType AttackType)
{

}

void UWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}
