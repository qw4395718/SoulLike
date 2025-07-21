// Fill out your copyright notice in the Description page of Project Settings.


#include "SoulLikeCharacter.h"
#include "CombatComponent.h"
#include "WeaponBase.h"

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

	//初始化组件
	CombatComponent = NewObject<UCombatComponent>(this,"CombatComponent");
	CombatComponent->RegisterComponent();
	CombatComponent->InitializeComponent();

	/*if (DefaultWeaponClass) {
		CombatComponent->EquipPrimaryWeapon(NewObject<UWeaponBase>(this, DefaultWeaponClass));
	}*/

	// 绑定武器切换事件
	//CombatComponent->OnWeaponChanged.AddUObject(this, &AALSCharacter::HandleWeaponChange);
	//绑定输入
	
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

void ASoulLikeCharacter::AttachWeaponToSocket(UWeaponBase* Weapon, FName SocketName)
{
	//检测武器是否已完成配置
	if (!Weapon || 
		(Weapon->IsStaticMesh == true && !Weapon->StaticWeaponMesh) ||
		(Weapon->IsStaticMesh == false && !Weapon->SkeletalWeaponMesh)) return;


	if (Weapon->IsStaticMesh == true)
	{
		Weapon->StaticWeaponMesh->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketName
		);
	}
	else
	{
		Weapon->SkeletalWeaponMesh->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			SocketName
		);
	}
}

FName ASoulLikeCharacter::GetWeaponHandSocket()
{
	return FName("");
}

bool ASoulLikeCharacter::CanAttack()
{
	return true;
}

void ASoulLikeCharacter::ConsumeStamina(float costAp)
{

}

