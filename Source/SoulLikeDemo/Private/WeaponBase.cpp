// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

void UWeaponBase::Initialize(ASoulLikeCharacter* Owner)
{
	OwningCharacter = Owner;

	// 创建武器网格体
	SkeletalWeaponMesh = NewObject<USkeletalMeshComponent>(Owner);
	SkeletalWeaponMesh->RegisterComponent();
	StaticWeaponMesh = NewObject<UStaticMeshComponent>(Owner);
	StaticWeaponMesh->RegisterComponent();

	// 获取ALS骨骼套接字
	FName SocketName = Owner->GetWeaponHandSocket();

	if (IsStaticMesh)
	{
		StaticWeaponMesh->AttachToComponent(
			Owner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketName
		);
	}
	else
	{
		SkeletalWeaponMesh->AttachToComponent(
			Owner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketName
		);
	}


	//// 类魂特性：绑定武器轨迹
	//SetupWeaponTrail();
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

float UWeaponBase::GetStaminaCost(EAttackType AttackType)
{
	return 0.0f;
}
