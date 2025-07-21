// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "SoulLikeCharacter.h"
#include "WeaponBase.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UCombatComponent::EquipPrimaryWeapon(UWeaponBase* NewWeapon)
{

}

void UCombatComponent::DrawWeapon(UWeaponBase* NewWeapon)
{
	if (EquippedWeapon) {
		SheathWeapon();
	}

	if (NewWeapon) {
		EquippedWeapon = NewWeapon;
		EquippedWeapon->Initialize(CharacterOwner);
	}
}

void UCombatComponent::SheathWeapon()
{

}

void UCombatComponent::PerformAttack(EAttackType AttackType)
{

}

void UCombatComponent::SwitchToWeapon(int32 Index)
{

}

bool UCombatComponent::CheckPerfectParry(float PlayerInputTime,float EnemyAttackTime)
{
	// 判定窗口：±0.3帧（60FPS下为5ms）
	const float ParryWindow = 0.005f;
	return FMath::Abs(PlayerInputTime - EnemyAttackTime) <= ParryWindow;
}

void UCombatComponent::ProcessAttackHit(AActor* HitActor, const FHitResult& HitResult) {
	if (!HitActor) return;

	// 检查是否可伤害
	if (IDamageable* Damageable = Cast<IDamageable>(HitActor)) {
		if (Damageable->CanReceiveDamage()) {
			// 创建伤害事件
			FDamageEventData DamageEvent;
			DamageEvent.BaseDamage = 10;
			DamageEvent.HitLocation = HitResult.Location;
			DamageEvent.bIsCriticalHit = false;
			DamageEvent.DamageCauser = GetOwner();

			// 类魂特性：武器类型影响
			/*if (CurrentWeapon) {
				DamageEvent.AttackType = CurrentWeapon->GetAttackType();
			}*/

			// 触发伤害事件
			Damageable->ReceiveDamage(DamageEvent);

			// 命中反馈
			//PlayHitEffect(DamageEvent.HitLocation);
		}
	}
}

void UCombatComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &UCombatComponent::StartAttack);
	//PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &UCombatComponent::StartHeavyAttack);
	//PlayerInputComponent->BindAction("Block", IE_Pressed, this, &UCombatComponent::StartBlocking);
}


void UCombatComponent::StartAttack()
{
	if (EquippedWeapon && CharacterOwner->CanAttack()) {
		EquippedWeapon->PlayAttackMontage(EAttackType::Normal_Combo_Phase_1);

		// 类魂特性：消耗耐力
		CharacterOwner->ConsumeStamina(EquippedWeapon->GetStaminaCost(EAttackType::Normal_Combo_Phase_1));
	}
}

void UCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 获取所属角色
	ASoulLikeCharacter* OwnerCharacter = Cast<ASoulLikeCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	// 初始化武器库存
	WeaponInventory.Empty(4); // 类魂标准4武器槽

	// 绑定输入
	SetupPlayerInput(OwnerCharacter->InputComponent);
}

