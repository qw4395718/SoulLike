// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "SoulLikeCharacter.h"
#include "WeaponBase.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	LH_EquippedWeapon = nullptr;
	RH_EquippedWeapon = nullptr;
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

	//默认初始化变量
	HealthPoint = 100.0f;
	ActionPoint = 100.0f;
	HealthPointMaxValue = 100.0f;
	ActionPointMaxValue = 100.0f;

	//将函数绑定到事件上
	DamageDispatcher->OnDamageEvent.AddDynamic(this, &UCombatComponent::HandleDamage);

	Initialize();
}



void UCombatComponent::Initialize()
{
	// 初始化左手武器
	if (LH_EquippedWeapon == nullptr)
	{
		LH_EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
		AWeaponBase::StaticClass(), FVector::ZeroVector,FRotator::ZeroRotator
		);
	}

	//将武器绑定到指定虚拟骨骼
	if (CharacterOwner->GetMesh()->DoesSocketExist(TEXT("ik_hand_l")) && 
		LH_EquippedWeapon)
	{
		LH_EquippedWeapon->AttachToComponent(
			CharacterOwner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
			TEXT("ik_hand_l") // Socket 名称
		);
		LH_EquippedWeapon->OwningCharacter = CharacterOwner;
	}

	// 初始化右手武器
	if (RH_EquippedWeapon == nullptr)
	{
		RH_EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
			AWeaponBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator
			);
	}

	//将武器绑定到指定虚拟骨骼
	if (CharacterOwner->GetMesh()->DoesSocketExist(TEXT("ik_hand_r")) &&
		RH_EquippedWeapon)
	{
		RH_EquippedWeapon->AttachToComponent(
			CharacterOwner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
			TEXT("ik_hand_r") // Socket 名称
		);
		RH_EquippedWeapon->OwningCharacter = CharacterOwner;
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

void UCombatComponent::PerformCombatSkill()
{
	if (LH_EquippedWeapon && CanAction()) {
		LH_EquippedWeapon->PerformCombatSkill();

		// 类魂特性：消耗耐力
		ChangeAP(LH_EquippedWeapon->GetStaminaCost(EAttackType::Skill_Combo_Phase_1));
	}
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

	//// 检查是否可伤害
	//if (IDamageable* Damageable = Cast<IDamageable>(HitActor)) {
	//	if (Damageable->CanReceiveDamage()) {
	//		// 创建伤害事件
	//		FDamageEventData DamageEvent;
	//		DamageEvent.BaseDamage = 10;
	//		DamageEvent.HitLocation = HitResult.Location;
	//		DamageEvent.bIsCriticalHit = false;
	//		DamageEvent.DamageCauser = GetOwner();

	//		// 类魂特性：武器类型影响
	//		/*if (CurrentWeapon) {
	//			DamageEvent.AttackType = CurrentWeapon->GetAttackType();
	//		}*/

	//		// 触发伤害事件
	//		Damageable->ReceiveDamage(DamageEvent);

	//		// 命中反馈
	//		//PlayHitEffect(DamageEvent.HitLocation);
	//	}
	//}
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
	////检查当前武器是否处于弹反窗口
	//if (RH_EquippedWeapon && RH_EquippedWeapon->IsParryWindowActive())
	//{	
	//	UE_LOG(LogTemp, Display, TEXT("Player Parryed"));
	//	// 被弹反成功,角色中断所有蒙太奇进入到待处决模式
	//	if (CharacterOwner)
	//	{
	//		CharacterOwner->PerformExecuted(FName(""));
	//	}
	//}
}

bool UCombatComponent::CanAction()
{
	return ActionPoint>0;
}

void UCombatComponent::ChangeAP(float CostNum)
{
	if (CostNum != 0 )
	{
		// 重置体力恢复定时器
		if (ReviveActionPointHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(ReviveActionPointHandle);
		}
		GetWorld()->GetTimerManager().SetTimer(
		ReviveActionPointHandle,
		this,
		&UCombatComponent::ReviveAP,
		TriggerReviveAPTimerInterval,
		true,
		EnableReviveAPTimerInterval);
	}

	if (ActionPoint + CostNum < 0)
	{
		// 执行完本次行动后进入力竭状态
		ActionPoint = 0;
	}
	else if(ActionPoint + CostNum > ActionPointMaxValue)
	{
		ActionPoint = ActionPointMaxValue;
	}
	else
	{
		ActionPoint += CostNum;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,               // Key（-1 表示不覆盖旧消息）
			5.0f,             // 显示时间（秒）
			FColor::Green,    // 颜色
			FString::Printf(TEXT("CurrentAP %f CostNum"), ActionPoint, CostNum) // 消息内容
		);
	}
}

void UCombatComponent::ReviveAP()
{
	// 根据属性确定恢复量
	// 当恢复满时,关闭定时器
	if (ActionPoint + APReviveValue >= ActionPointMaxValue)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReviveActionPointHandle);
	}
	else
	{
		ActionPoint += APReviveValue;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,               // Key（-1 表示不覆盖旧消息）
			5.0f,             // 显示时间（秒）
			FColor::Green,    // 颜色
			FString::Printf(TEXT("CurrentAP %f"), ActionPoint) // 消息内容
		);
	}
}

void UCombatComponent::PerformAttack()
{
	//// 根据当前条件确定战斗组件执行
	//// 进行一次球形检测,判断面前是否有敌人,根据敌人的当前状态(待处决)以及敌人的朝向和距离(可背刺)确定播放的蒙太奇动画
	//if(CharacterOwner == nullptr || RH_EquippedWeapon == nullptr) return;
	//
	//// 获取角色位置和前方向量
	//FVector CharacterLocation = CharacterOwner->GetActorLocation();
	//FVector CharacterForward = CharacterOwner->GetActorForwardVector();
	//FRotator CharacterRotator = CharacterOwner->GetActorRotation();

	//// 设置球形检测参数
	//TArray<AActor*> ActorsToIgnore;
	//ActorsToIgnore.Add(CharacterOwner); // 忽略自己

	//TArray<FHitResult> OutHits;
	//bool bHit = UKismetSystemLibrary::SphereTraceMulti(
	//	GetWorld(),
	//	CharacterLocation,
	//	CharacterLocation,
	//	DetectionRadius,
	//	UEngineTypes::ConvertToTraceType(ECC_Pawn), // 检测pawn类型
	//	false, // 不检测复杂碰撞
	//	ActorsToIgnore,
	//	EDrawDebugTrace::ForDuration, // 调试时显示，发布时可改为None
	//	OutHits,
	//	true
	//);

	//if (!bHit || OutHits.Num() == 0)
	//{
	//	// 若无符合的特殊攻击则进行普通攻击
	//	// 检查体力是否足够
	//	if (CanAction())
	//	{
	//		RH_EquippedWeapon->PerformAttack();
	//		// 类魂特性：消耗耐力
	//		ChangeAP(LH_EquippedWeapon->GetStaminaCost(EAttackType::Normal_Combo_Phase_1));

	//	}
	//	return;
	//}

	//// 遍历所有检测到的敌人
	//for (const FHitResult& Hit : OutHits)
	//{
	//	ASoulLikeCharacter* Enemy = Cast<ASoulLikeCharacter>(Hit.GetActor());

	//	if (Enemy /*&& Enemy->IsAlive()*/) // 确保是敌人且存活
	//	{
	//		// 检查敌人是否处于可处决状态
	//		if (Enemy->CanExecute())
	//		{
	//			RH_EquippedWeapon->PerformExecute();
	//			// 将敌人瞬移到角色面前指定位置
	//			FRotator NewRotator = CharacterRotator.Add(0,180,0);
	//			NewRotator.Normalize();
	//			Enemy->MoveToLocationAndRotation(
	//			CharacterLocation + CharacterForward * 100.0f,
	//			NewRotator);
	//			Enemy->PerformExecuted(FName("Executed_Sword"));
	//			return;
	//		}

	//		// 计算敌人位置和方向
	//		FVector EnemyLocation = Enemy->GetActorLocation();
	//		FVector EnemyForward = Enemy->GetActorForwardVector();

	//		// 计算玩家到敌人的向量
	//		FVector ToEnemy = EnemyLocation - CharacterLocation;
	//		float DistanceToEnemy = ToEnemy.Size();
	//		ToEnemy.Normalize();

	//		// 计算敌人后方角度
	//		float DotProduct = FVector::DotProduct(EnemyForward, ToEnemy);
	//		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	//		// 检查是否满足背刺条件
	//		if (DistanceToEnemy <= BackstabDistanceThreshold && Angle <= BackstabAngleThreshold)
	//		{
	//			RH_EquippedWeapon->PerformBackstab();
	//			Enemy->MoveToLocationAndRotation(
	//				CharacterLocation + CharacterForward * 100.0f,
	//				FRotator(CharacterRotator.Pitch, CharacterRotator.Yaw, CharacterRotator.Roll));
	//			Enemy->PerformBackStabbed();
	//			return;
	//		}
	//	}
	//}

	//// 若无符合的特殊攻击则进行普通攻击
	//if (CanAction())
	//{
	//	RH_EquippedWeapon->PerformAttack();
	//	// 类魂特性：消耗耐力
	//	ChangeAP(LH_EquippedWeapon->GetStaminaCost(EAttackType::Normal_Combo_Phase_1));

	//}
	//return;
}

void UCombatComponent::InitWeaponInventory(TArray<AWeaponBase*> arrWeaponInventory)
{

}

void UCombatComponent::WeaponInventoryChange(int32 Weaponindex, AWeaponBase* NewWeapon)
{

}
