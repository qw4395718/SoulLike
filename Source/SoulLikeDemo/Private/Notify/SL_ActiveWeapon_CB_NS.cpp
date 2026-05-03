// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_ActiveWeapon_CB_NS.h"
#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"
#include "SL_CharacterBase.h"

void USL_ActiveWeapon_CB_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		if (ASL_CharacterBase* Character = Cast<ASL_CharacterBase>(MeshComp->GetOwner()))
		{
			if (USL_EquipmentComponent* EquipmentComp = Cast<USL_EquipmentComponent>(Character->GetEquipmentComponent()))
			{
				if (ASL_WeaponBase* Weapon = EquipmentComp->GetCurrentLeftHandWeapon())
				{
					Weapon->EnableAttackCollision();
				}
				if (ASL_WeaponBase* Weapon = EquipmentComp->GetCurrentRightHandWeapon())
				{
					Weapon->EnableAttackCollision();
				}
			}
		}
	}
}

void USL_ActiveWeapon_CB_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色持有武器，并通过接口控制碰撞
		if (ASL_CharacterBase* Character = Cast<ASL_CharacterBase>(MeshComp->GetOwner()))
		{
			if (USL_EquipmentComponent* EquipmentComp = Cast<USL_EquipmentComponent>(Character->GetEquipmentComponent()))
			{
				if (ASL_WeaponBase* Weapon = EquipmentComp->GetCurrentLeftHandWeapon())
				{
					Weapon->DisableAttackCollision();
				}
				if (ASL_WeaponBase* Weapon = EquipmentComp->GetCurrentRightHandWeapon())
				{
					Weapon->DisableAttackCollision();
				}
			}
		}
	}
}
