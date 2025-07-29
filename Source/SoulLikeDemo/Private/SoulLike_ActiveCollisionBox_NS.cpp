// Fill out your copyright notice in the Description page of Project Settings.


#include "SoulLike_ActiveCollisionBox_NS.h"
#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"

void USoulLike_ActiveCollisionBox_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character && Character->CombatComponent && Character->CombatComponent->EquippedWeapon)
		{
			Character->CombatComponent->EquippedWeapon->EnableAttackCollisonCheck();
		}
	}
}

void USoulLike_ActiveCollisionBox_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		ASoulLikeCharacter* Character = Cast<ASoulLikeCharacter>(MeshComp->GetOwner());
		if (Character && Character->CombatComponent && Character->CombatComponent->EquippedWeapon)
		{
			Character->CombatComponent->EquippedWeapon->DisableAttackCollisonCheck();
		}
	}
}
