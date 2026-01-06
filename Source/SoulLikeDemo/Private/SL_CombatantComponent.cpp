// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CombatantComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <Engine/StreamableManager.h>
#include <Engine/AssetManager.h>
#include "GameFramework/Character.h"
#include <Delegates/DelegateSignatureImpl.inl>
#include "Health_IF.h"
#include "Stamina_IF.h"
#include <GameFramework/CharacterMovementComponent.h>
#include "StateCalculate_IF.h"
#include "BehavioralResponse_IF.h"
#include "WeaponBehavior_IF.h"
#include "CharacterComponent_IF.h"

USL_CombatantComponent::USL_CombatantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 默认初始化
	TeamID = 0;
	bWaitingForExecuted = false;
	bAllowedBackStabsed = false;
	SoftMentagePath = "";
	NeedPlayMetageSectionName = FName("");
	SoftMentageRefrence = nullptr;
	ActorOwner = nullptr;
}

void USL_CombatantComponent::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 直接使用生命接口,后续属性组件完成时，先由组件进行计算后再发给生命组件
	float CalculateDamage = Damage;
	if (IStateCalculate_IF* StateCalculateTarge = Cast<IStateCalculate_IF>(GetOwner()))
	{
		CalculateDamage = StateCalculateTarge->DamageReceivedCalculate(Damage);
	}

	if(IHealth_IF* HealthTarget = Cast<IHealth_IF>(GetOwner()))
	{
		HealthTarget->ReduceCurrentHealth(CalculateDamage);
		OnAttackEventCall();
	}
}

int USL_CombatantComponent::GetCurrentCombatState()
{
	return 0;
}

int USL_CombatantComponent::GetTeamID()
{
	return 0;
}

void USL_CombatantComponent::OnAttackEventCall()
{
	// 通知其他组件响应伤害事件
}

void USL_CombatantComponent::SetCanExecuteState(bool bCanExecuted)
{
	bWaitingForExecuted = bCanExecuted;
}

bool USL_CombatantComponent::CanExecute()
{
	return bWaitingForExecuted;
}

bool USL_CombatantComponent::CanBackStabs()
{
	return bAllowedBackStabsed;
}

void USL_CombatantComponent::PerformAttack()
{
	float StaminaCostValue = 20.0f;

	if (GetOwner() == nullptr) {return;}

	AActor* My = Cast<AActor>(GetOwner());
	if (My == nullptr) { return; }

	ICharacterComponent_IF* MyCharacterComponentTarget = Cast<ICharacterComponent_IF>(My);
	if (MyCharacterComponentTarget == nullptr) { return; }

	IWeaponBehavior_IF* MyWeaponBehavior = Cast<IWeaponBehavior_IF>(MyCharacterComponentTarget->GetEquipmentComponent());
	if(MyWeaponBehavior == nullptr){return;}

	IStamina_IF* StaminaTarget = Cast<IStamina_IF>(MyCharacterComponentTarget->GetStaminaComponent());
	// 检查是否处于精疲力竭状态
	if (StaminaTarget)
	{
		if (StaminaTarget->GetIsStaminaZero() == true)
		{
			return;
		}
	}

	// 获取角色位置和前方向量
	FVector CharacterLocation = GetOwner()->GetActorLocation();
	FVector CharacterForward = GetOwner()->GetActorForwardVector();
	FRotator CharacterRotator = GetOwner()->GetActorRotation();

	// 设置球形检测参数
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(ActorOwner); // 忽略自己

	TArray<FHitResult> OutHits;
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		CharacterLocation,
		CharacterLocation,
		DETECTION_RADIUS,
		UEngineTypes::ConvertToTraceType(ECC_Pawn), // 检测pawn类型
		false, // 不检测复杂碰撞
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration, // 调试时显示，发布时可改为None
		OutHits,
		true
	);

	if (!bHit || OutHits.Num() == 0)
	{
		// 若无符合的特殊攻击则进行普通攻击
		// 消耗体力
		IStateCalculate_IF* StateCalculateTarget = Cast<IStateCalculate_IF>(GetOwner());
		if (StateCalculateTarget)
		{
			StaminaCostValue = StateCalculateTarget->StaminaCostCalculate(StaminaCostValue);
		}
		StaminaTarget->ReduceStamina(StaminaCostValue);
	
		return;
	}

	// 遍历所有检测到的敌人
	for (const FHitResult& Hit : OutHits)
	{
		AActor* Enemy = Cast<AActor>(Hit.GetActor());
	
		if (Enemy) 
		{
			ICharacterComponent_IF* EnemyCharacterComponentTarget = Cast<ICharacterComponent_IF>(Enemy);
			if (EnemyCharacterComponentTarget == nullptr) { continue; }

			IHealth_IF* EnemyHealthTarget = Cast<IHealth_IF>(EnemyCharacterComponentTarget->GetHealthComponent());
			ICombat_IF* EnemyCombatTarget = Cast<ICombat_IF>(EnemyCharacterComponentTarget->GetCombatantComponent());
		
			// 是否可以进行下一步
			if (EnemyHealthTarget == nullptr ||
				EnemyCombatTarget == nullptr ||
				EnemyHealthTarget->IsAlive() == false)
			{
				continue;
			}

			// 检查敌人是否处于可处决状态
			if (EnemyCombatTarget->CanExecute())
			{
				//RH_EquippedWeapon->PerformExecute();
				// 将敌人瞬移到角色面前指定位置
				FRotator NewRotator = CharacterRotator.Add(0, 180, 0);
				NewRotator.Normalize();
				EnemyCombatTarget->MoveToLocationAndRotation(
					CharacterLocation + CharacterForward * 100.0f,
					NewRotator);
				EnemyCombatTarget->PerformExecuted(FName("Executed_Sword"));
				// 我方的武器接口
				MyWeaponBehavior->ExecuteBehaviorResponse(My);
				return;
			}

			// 计算敌人位置和方向
			FVector EnemyLocation = Enemy->GetActorLocation();
			FVector EnemyForward = Enemy->GetActorForwardVector();

			// 计算玩家到敌人的向量
			FVector ToEnemy = EnemyLocation - CharacterLocation;
			float DistanceToEnemy = ToEnemy.Size();
			ToEnemy.Normalize();

			// 计算敌人后方角度
			float DotProduct = FVector::DotProduct(EnemyForward, ToEnemy);
			float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

			// 检查是否满足背刺条件
			if (DistanceToEnemy <= BACKSTAB_DISTANCE_THRESHOLD && Angle <= BACKSTAB_ANGLE_THRESHOLD)
			{
				EnemyCombatTarget->MoveToLocationAndRotation(
					CharacterLocation + CharacterForward * 100.0f,
					FRotator(CharacterRotator.Pitch, CharacterRotator.Yaw, CharacterRotator.Roll));
				EnemyCombatTarget->PerformBackStabbed("BackStabbed_Sword");
				// 我方的武器接口
				MyWeaponBehavior->BackStabBehaviorResponse(My);
				return;
			}
		}
	}

	// 若无符合的特殊攻击则进行普通攻击
	MyWeaponBehavior->AttackBehaviorResponse(My);
	IStateCalculate_IF* StateCalculateTarget = Cast<IStateCalculate_IF>(My);
	if (StateCalculateTarget)
	{
		StaminaCostValue = StateCalculateTarget->StaminaCostCalculate(StaminaCostValue);
	}
	StaminaTarget->ReduceStamina(StaminaCostValue);
	return ;
}

void USL_CombatantComponent::PerformDefence()
{
	if (GetOwner() == nullptr) { return; }

	AActor* My = Cast<AActor>(GetOwner());
	if (My == nullptr) { return; }

	IWeaponBehavior_IF* MyWeaponBehavior = Cast<IWeaponBehavior_IF>(My);
	if (MyWeaponBehavior == nullptr) { return; }

	MyWeaponBehavior->DefenceBehaviorResponse(My);
	
	return;
}

void USL_CombatantComponent::PerformExecuted(FName MetageSectionName)
{
	AActor* OwnActor = GetOwner();
	if (OwnActor == nullptr) { return; }
	ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor);
	if (bWaitingForExecuted && OwnCharacter)
	{
		PlaySoftMentage(MetageSectionName);
	}
}

void USL_CombatantComponent::PerformBackStabbed(FName MetageSectionName)
{
	AActor* OwnActor = GetOwner();
	if (OwnActor == nullptr) { return; }
	ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor);
	if (bAllowedBackStabsed && OwnCharacter)
	{
		PlaySoftMentage(MetageSectionName);
	}
}

void USL_CombatantComponent::MoveToLocationAndRotation(FVector LocationPosition, FRotator Rotaion)
{
	AActor* OwnActor = GetOwner();
	if (OwnActor)
	{
		if (ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor))
		{
			OwnCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
			OwnCharacter->SetActorLocationAndRotation(LocationPosition, Rotaion, false, nullptr, ETeleportType::TeleportPhysics);
			// 恢复移动（如果是）
			OwnCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 恢复行走模式

		}
		else
		{
			OwnActor->SetActorLocationAndRotation(LocationPosition, Rotaion, false, nullptr, ETeleportType::TeleportPhysics);

		}

	}
}

void USL_CombatantComponent::InitCombatComponentInfo(AActor* OwnerActor, FString OwnerMentagePath, int OwnerTeamID, bool OwnerCanBackStab)
{
	ActorOwner = OwnerActor;
	TeamID = OwnerTeamID;
	bAllowedBackStabsed = OwnerCanBackStab;
	LoadActorMentageAsync(OwnerMentagePath);
}

void USL_CombatantComponent::PlaySoftMentage(FName MetageSectionName)
{
	// 0. 先检查软引用是否有效（路径是否正确）
	if (!SoftMentageRefrence.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SoftMontage path is not valid!"));
		return;
	}

	AActor* OwnActor = GetOwner();
	if (OwnActor == nullptr) { return; }

	// 1. 检查是否已经加载完成了（例如：之前已经加载过）
	if (UAnimMontage* LoadedMontage = SoftMentageRefrence.Get())
	{
		ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor);
		if (OwnCharacter)
		{
			UAnimInstance* AnimInstance = OwnCharacter->GetMesh()->GetAnimInstance();
			if (AnimInstance == nullptr){return;}
			if(AnimInstance->Montage_IsActive(LoadedMontage) &&
			LoadedMontage->IsValidSectionName(MetageSectionName))
			{
				AnimInstance->Montage_JumpToSection(MetageSectionName);
			}
			else if(LoadedMontage->IsValidSectionName(MetageSectionName))
			{
				AnimInstance->Montage_Play(LoadedMontage);
				AnimInstance->Montage_JumpToSection(MetageSectionName, LoadedMontage);
			}
			else
			{
				// 不展示动画
			}
		}
		return;
	}

	// 临时存储需要播放的蒙太奇片段名称
	NeedPlayMetageSectionName = MetageSectionName;

	// 2. 如果还未加载，则发起异步加载请求
	//LoadActorMentageAsync();

	//// 可选的：设置加载优先级和参数
	//MontageStreamableHandle = Streamable.RequestAsyncLoad(
	//	SoftMentageRefrence.ToSoftObjectPath(),
	//	Delegate
	//	//, FStreamableManager::DefaultAsyncLoadPriority, false
	//);

	// 此时，你可以在这里更新UI，比如显示一个“加载中”的提示
	UE_LOG(LogTemp, Log, TEXT("Started async loading montage..."));
}

void USL_CombatantComponent::LoadActorMentageAsync(const FString MentagePath)
{
	if (MentagePath == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMentageRefrence = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(*MentagePath));
	Streamable.RequestAsyncLoad(
		SoftMentageRefrence.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &USL_CombatantComponent::OnActorMentageLoaded)
	);
}

void USL_CombatantComponent::OnActorMentageLoaded()
{
	// 打印信息
	UE_LOG(LogTemp,Log,TEXT("ZYF_ALS_USL_CombatantComponent::OnActorMentageLoaded()"));
	// 3.1 通过句柄检查（推荐）
	//if (MontageStreamableHandle.IsValid() && MontageStreamableHandle->HasLoadCompleted())
	//{
	//	// 从软引用中获取已经加载完成的资源对象
	//	UAnimMontage* LoadedMontage = SoftMentageRefrence.Get();
	//	// 3.2 再次检查获取到的对象是否有效
	//	if (LoadedMontage)
	//	{
	//		// 4. 资源加载成功，使用它！
	//		PlaySoftMentage(NeedPlayMetageSectionName);
	//		UE_LOG(LogTemp, Log, TEXT("Montage loaded and played successfully!"));
	//	}
	//	else
	//	{
	//		// 理论上不应该走到这里，除非资源加载后又被强制卸载了
	//		UE_LOG(LogTemp, Error, TEXT("Montage failed to load: Get() returned nullptr after successful load."));
	//	}

	//	// 释放句柄引用。注意：这不会卸载资源，因为MySkeletalMeshComponent现在持有对它的强引用（PlayAnimMontage内部会设置）
	//	MontageStreamableHandle->ReleaseHandle();
	//	MontageStreamableHandle.Reset();
	//}
	//else
	//{
	//	// 5. 处理加载失败的情况
	//	UE_LOG(LogTemp, Error, TEXT("Failed to load montage at path: %s"), *LoadedPath.ToString());
	//	// 这里可以给玩家一个反馈，比如播放一个默认动画或显示错误信息

	//	// 确保清理句柄
	//	if (MontageStreamableHandle.IsValid())
	//	{
	//		MontageStreamableHandle->ReleaseHandle();
	//		MontageStreamableHandle.Reset();
	//	}
	//}
}

