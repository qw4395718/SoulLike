// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

void UWeaponBase::Initialize(ASoulLikeCharacter* Owner)
{
	
}

FDamageData UWeaponBase::GetDamageData_Implementation() const
{
	return FDamageData();
}

void UWeaponBase::PlayAttackMontage_Implementation(EAttackType AttackType)
{
	if (AttackMontages.Find(AttackType) != nullptr && OwningCharacter)
	{
		UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		if(!AnimInstance)return;
		AnimInstance->Montage_Play(*AttackMontages.Find(AttackType));
	}
}

void UWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

float UWeaponBase::GetStaminaCost(EAttackType AttackType)
{
	return 0.0f;
}
