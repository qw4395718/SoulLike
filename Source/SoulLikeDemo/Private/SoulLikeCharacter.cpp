// Fill out your copyright notice in the Description page of Project Settings.


#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"

// Sets default values
ASoulLikeCharacter::ASoulLikeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 初始化组件
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// 白盒初始化
	Initialize();
	
}

void ASoulLikeCharacter::PerformAttack()
{
	CombatComponent->PerformAttack();
}

void ASoulLikeCharacter::PerformCombatSkill()
{
	CombatComponent->PerformCombatSkill();
}

// Called when the game starts or when spawned
void ASoulLikeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 绑定自身伤害处理
	CombatComponent->InitializeComponent();
}

// Called every frame
void ASoulLikeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASoulLikeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASoulLikeCharacter::PerformAttack);
	PlayerInputComponent->BindAction("CombatSkill", IE_Pressed, this, &ASoulLikeCharacter::PerformCombatSkill);

}

void ASoulLikeCharacter::Initialize()
{
	// 资产初始化
	// 加载武器动画资源
	UAnimMontage* Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_Character_Backstabbed.AM_Character_Backstabbed")
		);
	if (Montage)
	{
		BackStabbedMontage = Montage;
	}

	// 加载武器动画资源
	Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_Character_Executed.AM_Character_Executed")
		);
	if (Montage)
	{
		ExecutionedMontage = Montage;
	}
}


void ASoulLikeCharacter::PlayBackStabbedMontage()
{
	// 直接播放蒙太奇动画，后续补充状态判断
	UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(BackStabbedMontage);
	}
}

void ASoulLikeCharacter::PlayExecutionedMontage()
{
	// 直接播放蒙太奇动画，后续补充状态判断
	UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(ExecutionedMontage);
	}
}
