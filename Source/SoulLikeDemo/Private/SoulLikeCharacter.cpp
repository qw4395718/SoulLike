// Fill out your copyright notice in the Description page of Project Settings.


#include "SoulLikeCharacter.h"

// Sets default values
ASoulLikeCharacter::ASoulLikeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASoulLikeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 创建事件分发器
	DamageDispatcher = NewObject<UDamageEventDispatcher>(this);

	// 绑定自身伤害处理
	DamageDispatcher->OnDamageEvent.AddDynamic(this, &ASoulLikeCharacter::HandleDamage);
	
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

}

void ASoulLikeCharacter::ReceiveDamage_Implementation(const FDamageEventData& DamageEvent)
{
	if (CanReceiveDamage()) {
		//广播给所有FOnDamageSignature
		DamageDispatcher->BroadcastDamageEvent(DamageEvent);
	}
}

bool ASoulLikeCharacter::CanReceiveDamage_Implementation() const
{//判定当前状态能否受到伤害
	return true;
}

void ASoulLikeCharacter::ReducePoise(float Amount)
{
}

void ASoulLikeCharacter::HandleDamage(const FDamageEventData& DamageEvent)
{
	//根据传入数据和玩家自身数据确定最终伤害
}

