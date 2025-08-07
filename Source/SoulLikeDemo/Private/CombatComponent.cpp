// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "SoulLikeCharacter.h"
#include "WeaponBase.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	EquippedWeapon = nullptr;
	// ...
	// 创建事件分发器
	DamageDispatcher = nullptr;
}

void UCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 获取所属角色
	CharacterOwner = Cast<ASoulLikeCharacter>(GetOwner());
	if (!CharacterOwner) return;

	if (DamageDispatcher == nullptr)
	{
		DamageDispatcher = NewObject<UDamageEventDispatcher>(this,TEXT("DamageDispatcher"));
	}

	// 初始化武器库存
	WeaponInventory.Empty(4); // 类魂标准4武器槽

	// 绑定输入
	SetupPlayerInput(CharacterOwner->InputComponent);

	//默认初始化变量
	HealthPoint = 100.0f;
	ActionPoint = 100.0f;

	//将函数绑定到事件上
	DamageDispatcher->OnDamageEvent.AddDynamic(this, &UCombatComponent::HandleDamage);

	Initialize();
}



void UCombatComponent::Initialize()
{
	if (EquippedWeapon == nullptr)
	{
		EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
		AWeaponBase::StaticClass(), FVector::ZeroVector,FRotator::ZeroRotator
		);
	}

	//将武器绑定到指定虚拟骨骼
	if (CharacterOwner->GetMesh()->DoesSocketExist(TEXT("VB RHS_ik_hand_gun")) && 
	EquippedWeapon)
	{
		EquippedWeapon->AttachToComponent(
			CharacterOwner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
			TEXT("VB RHS_ik_hand_gun") // Socket 名称
		);
		EquippedWeapon->OwningCharacter = CharacterOwner;
	}

}

void UCombatComponent::DrawWeapon()
{
	//if (EquippedWeapon == nullptr) 
	//{//如果当前未有武器装备,则从武器仓库中拿一把
	//	bool bFindValidWeapon = false;
	//	for each (AWeaponBase* pWeapon in WeaponInventory)
	//	{
	//		if (pWeapon != nullptr)
	//		{
	//			EquippedWeapon = pWeapon;
	//			bFindValidWeapon = true;
	//			break;
	//		}
	//		if (bFindValidWeapon == true)
	//		{
	//			
	//		}
	//		else
	//		{
	//			//无有效的武器
	//		}
	//	}
	//}

}

void UCombatComponent::SheathWeapon()
{

}

void UCombatComponent::PerformAttack()
{
	StartAttack();
}

void UCombatComponent::PerformCombatSkill()
{
	StartCombatSkill();
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

void UCombatComponent::HandleDamage(const FDamageEventData& DamageEvent)
{
	//处理伤害
	float ResultHP = 0;
	ResultHP = HealthPoint - DamageEvent.BaseDamage > 0? HealthPoint - DamageEvent.BaseDamage : 0;
	
	if (ResultHP == 0)
	{
		HealthPoint = 0;
		//播放角色死亡动画
		UE_LOG(LogTemp, Display, TEXT("Player Die"));
	}
	else
	{
		HealthPoint = ResultHP;
		//播放角色受击动画
		UE_LOG(LogTemp, Display, TEXT("Player Currnet HP:%f"), HealthPoint);
	}
}

void UCombatComponent::HandleParry()
{
	//检查当前武器是否处于弹反窗口
	if (EquippedWeapon && EquippedWeapon->IsParryWindowActive())
	{	
		// 被弹反成功,角色中断所有动画进入到待处决模式
		UE_LOG(LogTemp, Display, TEXT("Player Parryed"));
	}
}

void UCombatComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	//PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &UCombatComponent::StartAttack);
}

void UCombatComponent::StartAttack()
{
	if (EquippedWeapon /*&& CanAttack()*/) {
		EquippedWeapon->PerformAttack();

		// 类魂特性：消耗耐力
		//CharacterOwner->ConsumeStamina(EquippedWeapon->GetStaminaCost(EAttackType::Normal_Combo_Phase_1));
	}
}


void UCombatComponent::StartCombatSkill()
{
	if (EquippedWeapon /*&& CanAttack()*/) {
		EquippedWeapon->PerformCombatSkill();

		// 类魂特性：消耗耐力
		//CharacterOwner->ConsumeStamina(EquippedWeapon->GetStaminaCost(EAttackType::Normal_Combo_Phase_1));
	}
}

void UCombatComponent::InitWeaponInventory(TArray<AWeaponBase*> arrWeaponInventory)
{

}

void UCombatComponent::WeaponInventoryChange(int32 Weaponindex, AWeaponBase* NewWeapon)
{

}

void UCombatComponent::ReceiveDamage_Implementation(const FDamageEventData& DamageEvent)
{
	if (CanReceiveDamage()) {
		//广播给所有FOnDamageSignature
		DamageDispatcher->BroadcastDamageEvent(DamageEvent);
	}
}

bool UCombatComponent::CanReceiveDamage_Implementation() const
{//判定当前状态能否受到伤害
	return true;
}