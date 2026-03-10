// Fill out your copyright notice in the Description page of Project Settings.


#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Engine/DataTable.h>
#include "WeaponTableInfo.h"


// Sets default values
ASoulLikeCharacter::ASoulLikeCharacter()
{
}


void ASoulLikeCharacter::PerformAttack()
{
	if (
		bIsExecuting == false &&
		bIsBackStabbing == false 
		)
	{
		CombatComponent->PerformAttack();
	}
}

void ASoulLikeCharacter::PerformCombatSkill()
{
	if (
		bIsExecuting == false &&
		bIsBackStabbing == false 
		)
	{
		CombatComponent->PerformCombatSkill();
	}
}

// Called when the game starts or when spawned
void ASoulLikeCharacter::BeginPlay()
{
	Super::BeginPlay();

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

void ASoulLikeCharacter::PlayExecutionedMontage(FName MontageSectionName)
{
	// 直接播放蒙太奇动画，后续补充状态判断
	UAnimInstance* AnimInstance = this->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (AnimInstance->Montage_IsActive(ExecutionedMontage) &&
			MontageSectionName != FName(""))
		{
			AnimInstance->Montage_JumpToSection(MontageSectionName);
		}
		else 
		{
			AnimInstance->Montage_Play(ExecutionedMontage);
		}
		
	}
}

void ASoulLikeCharacter::ReadTableTest(const FString TablePath)
{
	// 
	UDataTable* MyDataTable = LoadObject<UDataTable>(nullptr, *TablePath);

	if (MyDataTable)
	{
		for (FName RowName : MyDataTable->GetRowNames())
		{
			FWeaponTableInfo* RowData = MyDataTable->FindRow<FWeaponTableInfo>(RowName, TEXT("WeaponID"));
			UE_LOG(LogTemp, Display, TEXT("%d"), RowData->WeaponID);
		}
	}
}



void ASoulLikeCharacter::SetExecutingState(bool fese)
{

}

void ASoulLikeCharacter::MoveToLocationAndRotation(FVector LocationPosition,FRotator Rotaion)
{
	this->GetCharacterMovement()->SetMovementMode(MOVE_None);
	this->SetActorLocationAndRotation(LocationPosition, Rotaion, false, nullptr, ETeleportType::TeleportPhysics);
	// 恢复移动（如果是）
	this->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 恢复行走模式
}



