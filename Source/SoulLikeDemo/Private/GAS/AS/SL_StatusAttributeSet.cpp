#include <SL_StatusAttributeSet.h>
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>
#include <GlobalDelegatesManager.h>
#include <SL_Macros.h>
#include <SL_CharacterBase.h>
#include "SL_EnemyBase.h"
#include <Components/CapsuleComponent.h>

#include "Stats/Stats.h"

DECLARE_CYCLE_STAT(TEXT("SL_StatusAttributeSet_PostGameplayEffectExecute"), STAT_SL_StatusAttributeSet_PostGameplayEffectExecute, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("SL_StatusAttributeSet_PostGameplayEffectExecute_Damage"), STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_Damage, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost"), STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost_ExecuteGameplayCue"), STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost_ExecuteGameplayCue, STATGROUP_Game);

USL_StatusAttributeSet::USL_StatusAttributeSet()
{
	// 构造阶段没有有效的 World 上下文，Cook 时会导致 Cannot get World 错误
}

void USL_StatusAttributeSet::SetOwningActor(AActor* pOwnActor)
{
	RETURN_IF_TRUE(pOwnActor == nullptr);
	OwningActor = pOwnActor;
}

/************************************************************************/
/*                               外部调用                                */
/************************************************************************/
void USL_StatusAttributeSet::InitHealthAS_Implementation(float MinValue, float MaxValue)
{
	InitHealth(MaxValue);
	InitMaxHealth(MaxValue);
	InitDamage(0.0f);

	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, MaxValue, MaxValue, MinValue, MaxValue);
	}
}

void USL_StatusAttributeSet::InitStaminaAS_Implementation(float MinValue, float MaxValue)
{
	InitStamina(MaxValue);      // 满耐力
	InitMaxStamina(MaxValue);
	InitStaminaCost(0.0f);      // 清空meta属性
	InitStaminaRegen(0.0f);     // 清空meta属性
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		globalDelegatesManager->OnAttributeStaminaChanged.Broadcast(OwningActor, MaxValue, MaxValue, MinValue, MaxValue);
	}
}

void USL_StatusAttributeSet::OnRep_CurrentHealth()
{
	if (OwningActor != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentHealth: OwningActor:%s Role:%d Attribute -> %.2f"), *(OwningActor->GetName()), (int)(OwningActor->GetLocalRole()), Health.GetCurrentValue());
		if (ASL_CharacterBase* actor = Cast<ASL_CharacterBase>(OwningActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentHealth: OwningActorNGUID:%s "), *(actor->GetNetworkGUIDString(actor)));
		}
		const float DamageValue = GetDamage();
		const float OldHealthValue = GetHealth();
		const float MinHealthValue = 0.0f;
		const float MaxHealthValue = GetMaxHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue - DamageValue, MinHealthValue, MaxHealthValue);

		SetHealth(NewHealthValue);
		const float CurrentHealth = GetHealth();
		if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
		{
			globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldHealthValue, CurrentHealth, MinHealthValue, MaxHealthValue);
		}
	}
}

void USL_StatusAttributeSet::OnRep_CurrentStamina()
{
	if (OwningActor != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentStamina: OwningActor:%s Role:%d Attribute -> %.2f"), *(OwningActor->GetName()), (int)(OwningActor->GetLocalRole()), Stamina.GetCurrentValue());
		if (ASL_CharacterBase* actor = Cast<ASL_CharacterBase>(OwningActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentStamina: OwningActorNGUID:%s "), *(actor->GetNetworkGUIDString(actor)));
		}
		const float StaminaCostValue = GetStaminaCost();
		const float OldStaminaValue = GetStamina();
		const float MinStaminaValue = 0.0f;
		const float MaxStaminaValue = GetMaxStamina();
		const float NewStaminaValue = FMath::Clamp(OldStaminaValue - StaminaCostValue, MinStaminaValue, MaxStaminaValue);

		SetStamina(NewStaminaValue);
		const float CurrentStamina = GetStamina();
		if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
		{
			globalDelegatesManager->OnAttributeStaminaChanged.Broadcast(OwningActor, OldStaminaValue, CurrentStamina, MinStaminaValue, MaxStaminaValue);
		}
	}
}

void USL_StatusAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	//UE_LOG(LogTemp, Warning, TEXT("PreChange: Attribute '%s'"), *Attribute.AttributeName);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	Super::PreAttributeChange(Attribute, NewValue);
}

void USL_StatusAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	SCOPE_CYCLE_COUNTER(STAT_SL_StatusAttributeSet_PostGameplayEffectExecute);
	Super::PostGameplayEffectExecute(Data);
	//UE_LOG(LogTemp, Warning, TEXT("PostChange: Attribute -> %.2f"), Data.EvaluatedData.Magnitude);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SCOPE_CYCLE_COUNTER(STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_Damage);
		// Convert into -Health and then clamp
		const float ChangeValue = GetDamage();
		const float OldValue = GetHealth();
		const float MinValue = 0.0f;
		const float MaxValue = GetMaxHealth();
		const float NewValue = FMath::Clamp(OldValue - ChangeValue, MinValue, MaxValue);

		if (OldValue != NewValue)
		{
			// Set the new health after clamping to min-max
			SetHealth(NewValue);
			const float CurrentValue = GetHealth();
			UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this);
			if (globalDelegatesManager != nullptr)
			{
				globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
				// 血量归零
				if (CurrentValue == MinValue) {OnCharacterDeath(OwningActor);}
			}

			// Calculate 'actual' damage applied that respects min and max health
			const float ChangeNumber = OldValue - NewValue;

			// 广播伤害飘字
			if (ChangeNumber > 0.0f && globalDelegatesManager != nullptr)
			{
				FDamageFloatingTextData TextData;
				TextData.DamageValue = ChangeNumber;
				TextData.TargetActor = OwningActor;

				// 从 GE 上下文中提取受击位置
				if (const FHitResult* HitResult = Data.EffectSpec.GetContext().GetHitResult())
				{
					TextData.HitWorldLocation = HitResult->Location;
				}
				else
				{
					TextData.HitWorldLocation = OwningActor->GetActorLocation();
				}

				// 暴击判定：通过 GE 携带的 Tag "Combat.CriticalHit" 识别
				TextData.bIsCriticalHit = true/*Data.EffectSpec.CapturedSourceTags.GetAggregatedTags()->HasTag(
					FGameplayTag::RequestGameplayTag(FName("Combat.CriticalHit")))*/;

				globalDelegatesManager->BroadcastDamageFloatingText(TextData);
			}

			if (UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent())
			{	
				// 在PIE阶段,这个是运行时进行编译,所以会导致卡顿,打包后不会发生
				// const FGameplayTag ChangeCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.DamageNumber"));
				// FGameplayCueParameters ChangeCueParams;
				// ChangeCueParams.NormalizedMagnitude = 1.f;
				// ChangeCueParams.RawMagnitude = ChangeNumber;
				// OwningAbilitySystemComponent->ExecuteGameplayCue(ChangeCueTag, ChangeCueParams);
			}
		}

		// Clear the meta attribute that temporarily held damage
		SetDamage(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaCostAttribute())
	{
		SCOPE_CYCLE_COUNTER(STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost);
		// Convert into -Health and then clamp
		const float ChangeValue = GetStaminaCost();
		const float OldValue = GetStamina();
		const float MinValue = 0.0f;
		const float MaxValue = GetMaxStamina();
		const float NewValue = FMath::Clamp(OldValue - ChangeValue, MinValue, MaxValue);

		if (OldValue != NewValue)
		{
			// Set the new health after clamping to min-max
			SetStamina(NewValue);
			const float CurrentValue = GetStamina();
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeStaminaChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
			}

			// Calculate 'actual' damage applied that respects min and max health
			const float ChangeNumber = OldValue - NewValue;
			if (UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent())
			{
				// 在PIE阶段,这个是运行时进行编译,所以会导致卡顿,打包后不会发生
				//SCOPE_CYCLE_COUNTER(STAT_SL_StatusAttributeSet_PostGameplayEffectExecute_StaminaCost_ExecuteGameplayCue);
				//// Broadcast a 'damage number' gameplay cue on the owning actor. Triggered on server, executes on all clients.
				//const FGameplayTag ChangeCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.StaminaCostNumber"), /*ErrorIfNotFound=*/true);
				//FGameplayCueParameters ChangeCueParams;
				//ChangeCueParams.NormalizedMagnitude = 1.f;
				//ChangeCueParams.RawMagnitude = ChangeValue;
				//OwningAbilitySystemComponent->ExecuteGameplayCue(ChangeCueTag, ChangeCueParams);
			}
		}

		// Clear the meta attribute that temporarily held damage
		SetStaminaCost(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaRegenAttribute())
	{
		// 恢复Meta属性 → 转换为耐力增加
		const float RegenValue = GetStaminaRegen();
		if (RegenValue > 0.0f)
		{
			const float OldValue = GetStamina();
			const float NewValue = FMath::Min(OldValue + RegenValue, GetMaxStamina());

			if (OldValue != NewValue)
			{
				SetStamina(NewValue);
				const float CurrentValue = GetStamina();

				if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
				{
					globalDelegatesManager->OnAttributeStaminaChanged.Broadcast(OwningActor, OldValue, CurrentValue, 0.0f, GetMaxStamina());
				}
			}
		}
		SetStaminaRegen(0.0f);  // 清空Meta属性
	}
}

// ===== 新增死亡处理函数 =====
void USL_StatusAttributeSet::OnCharacterDeath(AActor* DeathActor)
{
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    if (!ASC) return;

    AActor* OwnerActor = ASC->GetOwnerActor();
    if (!OwnerActor || DeathActor != OwnerActor) return;

    // 1. 更新GAS标签
    ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Alive"));
    ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));

    // 2. 广播死亡事件
    if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
    {
        DelegateMgr->OnCharacterDied.Broadcast(OwnerActor, nullptr);
    }

    UE_LOG(LogTemp, Warning, TEXT("GAS Death: %s died"), *OwnerActor->GetName());
}


void USL_StatusAttributeSet::OnCharacterReLive(AActor* ReviveActor)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	AActor* OwnerActor = ASC->GetOwnerActor();
	if (!OwnerActor|| ReviveActor != OwnerActor) return;

	// 1. 更新GAS标签
	ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Alive")); 

	 // 2. 广播复活事件
    if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
    {
        DelegateMgr->OnCharacterRevived.Broadcast(OwnerActor);
    }
}

void USL_StatusAttributeSet::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USL_StatusAttributeSet, MaxHealth);
	DOREPLIFETIME(USL_StatusAttributeSet, Health);
	DOREPLIFETIME(USL_StatusAttributeSet, MaxStamina);
	DOREPLIFETIME(USL_StatusAttributeSet, Stamina);
}