// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_ActiveWeapon_CB_NS.h"
#include "SL_CharacterBase.h"
#include <SL_WeaponBase.h>

void USL_ActiveWeapon_CB_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色/NPC持有武器，并通过接口控制碰撞
		if (IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(MeshComp->GetOwner()))
		{
			if (ASL_WeaponBase* Weapon = WeaponAccessory->GetLeftHandWeapon())
			{
				Weapon->EnableAttackCollision();
			}
			if (ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon())
			{
				Weapon->EnableAttackCollision();
			}
			
		}
	}
}

void USL_ActiveWeapon_CB_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		// 假设角色/NPC持有武器，并通过接口控制碰撞
		if (IWeaponAccessory_IF* WeaponAccessory = Cast<IWeaponAccessory_IF>(MeshComp->GetOwner()))
		{
			if (ASL_WeaponBase* Weapon = WeaponAccessory->GetLeftHandWeapon())
			{
				Weapon->DisableAttackCollision();
			}
			if (ASL_WeaponBase* Weapon = WeaponAccessory->GetRightHandWeapon())
			{
				Weapon->DisableAttackCollision();
			}
			
		}
	}
}
