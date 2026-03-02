// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_ActiveWeapon_CB_NS.h"
#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"

void USL_ActiveWeapon_CB_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character && Character->CombatComponent && Character->CombatComponent->LH_EquippedWeapon && bIsLHActive)
		{
			Character->CombatComponent->LH_EquippedWeapon->EnableAttackCollisonCheck();
		}
		if (Character && Character->CombatComponent && Character->CombatComponent->RH_EquippedWeapon && bIsRHActive)
		{
			Character->CombatComponent->RH_EquippedWeapon->EnableAttackCollisonCheck();
		}
	}
}

void USL_ActiveWeapon_CB_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character && Character->CombatComponent && Character->CombatComponent->LH_EquippedWeapon && bIsLHActive)
		{
			Character->CombatComponent->LH_EquippedWeapon->DisableAttackCollisonCheck();
		}
		if (Character && Character->CombatComponent && Character->CombatComponent->RH_EquippedWeapon && bIsRHActive)
		{
			Character->CombatComponent->RH_EquippedWeapon->DisableAttackCollisonCheck();
		}
	}
}
