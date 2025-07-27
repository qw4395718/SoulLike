// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"

AWeaponBase::AWeaponBase()
{
	//是否开启tick
	PrimaryActorTick.bCanEverTick = true;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	// 场景组件root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;
	// 胶囊体-大小根据实际skelete或者staticmesh大小确定
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(RootComponent);

	// 骨骼网格体-
	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalWeaponMesh->SetupAttachment(RootComponent);
	// 静态网格体-
	StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticWeaponMesh->SetupAttachment(RootComponent);

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/
	// 默认初始化
	bEnableCapsuleCheck = false;
	IsStaticMesh = true;
	WeaponData.WeaponCollisonHalfHeight = 55.0f;
	WeaponData.WeaponCollisonRadius = 5.0f;
	CapsuleComp->SetCapsuleSize(WeaponData.WeaponCollisonRadius, WeaponData.WeaponCollisonHalfHeight);

}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//检测是否开启了碰撞
	if (bEnableCapsuleCheck)
	{
		CheckHit();
	}

}

void AWeaponBase::Initialize()
{
	
}

FDamageData AWeaponBase::GetDamageData_Implementation() const
{
	return FDamageData();
}

void AWeaponBase::SetEnableCapsuleCheck(bool enable)
{
	bEnableCapsuleCheck = enable;
}

void AWeaponBase::CheckHit()
{
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * WeaponData.WeaponCollisonHalfHeight*2;

	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn))
	{
		// 处理命中逻辑
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 检查是否是特定类
			ASoulLikeCharacter* Enemy = Cast<ASoulLikeCharacter>(HitActor);
			if (Enemy && GetOwner() != Enemy)
			{
				// 处理敌人命中逻辑
				FDamageEventData DamageEventData;
				DamageEventData.BaseDamage = WeaponData.BaseDamage;
				DamageEventData.HitLocation = HitResult.Location;
				DamageEventData.HitNormal = HitResult.Normal;
				DamageEventData.bIsCriticalHit = false;
				DamageEventData.AttackType = EDamageType::SLASH;
				DamageEventData.DamageCauser = GetOwner();
		
				Enemy->CombatComponent->HandleDamage(DamageEventData);
			}
		}
	}

}

void AWeaponBase::PlayAttackMontage_Implementation(EAttackType AttackType)
{
	if (AttackMontages.Find(AttackType) != nullptr && OwningCharacter)
	{
		UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		if(!AnimInstance)return;
		AnimInstance->Montage_Play(*AttackMontages.Find(AttackType));
	}
}

void AWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

float AWeaponBase::GetStaminaCost(EAttackType AttackType)
{
	return 0.0f;
}
