// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"

void UMeleeWeapon::Initialize(ASoulLikeCharacter* OwnerCharacter)
{
	if (!OwnerCharacter) {
		UE_LOG(LogTemp, Error, TEXT("Invalid owner character for weapon initialization"));
		return;
	}

	//保存角色引用
	OwningCharacter = OwnerCharacter;

	// 创建武器网格体组件
	if (IsStaticMesh)
	{
		SkeletalWeaponMesh = NewObject<USkeletalMeshComponent>(OwnerCharacter);
		SkeletalWeaponMesh->SetupAttachment(OwnerCharacter->GetMesh(), "Weapon_Socket");
		SkeletalWeaponMesh->RegisterComponent();
	}
	else
	{
		StaticWeaponMesh = NewObject<UStaticMeshComponent>(OwnerCharacter);
		StaticWeaponMesh->SetupAttachment(OwnerCharacter->GetMesh(), "Weapon_Socket");
		StaticWeaponMesh->RegisterComponent();
	}
}

float UMeleeWeapon::GetStaminaCost(EAttackType AttackType)
{
	//后续改为读取配置返回指定的体力消耗
	return 10.0f;
}

void UMeleeWeapon::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}
