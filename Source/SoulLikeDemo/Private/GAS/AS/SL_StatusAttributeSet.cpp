#include <SL_StatusAttributeSet.h>
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>
#include <GlobalDelegatesManager.h>
#include <SL_Macros.h>
#include <SL_CharacterBase.h>

USL_StatusAttributeSet::USL_StatusAttributeSet() 
{
}

void USL_StatusAttributeSet::SetOwningActor(AActor* pOwnActor)
{
	RETURN_IF_TRUE(pOwnActor == nullptr);
	OwningActor = pOwnActor;
}

void USL_StatusAttributeSet::InitStatusAS_Implementation()
{
	// 由Unlua提供覆盖,C++中仅提供默认设置
	InitHealthAS(0.0f,100.0f);
	InitStaminaAS(0.0f, 100.0f);
}

void USL_StatusAttributeSet::InitHealthAS_Implementation(float MinValue, float MaxValue)
{
	InitHealth(MaxValue);
	InitMaxHealth(MaxValue);
	// 添加到全局委托的响应
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, MaxValue, MaxValue, MinValue, MaxValue);
	}
}

void USL_StatusAttributeSet::InitStaminaAS_Implementation(float MinValue, float MaxValue)
{
	InitStamina(MaxValue);
	InitMaxStamina(MaxValue);
	// 添加到全局委托的响应
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
		// 添加到全局委托的响应
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
		// 添加到全局委托的响应
		if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
		{
			globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldStaminaValue, CurrentStamina, MinStaminaValue, MaxStaminaValue);
		}
	}
}

void USL_StatusAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	UE_LOG(LogTemp, Warning, TEXT("PreChange: Attribute '%s'"), *Attribute.AttributeName);
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
	Super::PostGameplayEffectExecute(Data);
	UE_LOG(LogTemp, Warning, TEXT("PostChange: Attribute -> %.2f"), Data.EvaluatedData.Magnitude);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 生命值不直接进行修改
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
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
			// 添加到全局委托的响应
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
			}

			// Calculate 'actual' damage applied that respects min and max health
			const float ChangeNumber = OldValue - NewValue;
			if (UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent())
			{
				// Broadcast a 'damage number' gameplay cue on the owning actor. Triggered on server, executes on all clients.
				const FGameplayTag ChangeCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.DamageNumber"), /*ErrorIfNotFound=*/true);
				FGameplayCueParameters ChangeCueParams;
				ChangeCueParams.NormalizedMagnitude = 1.f;
				ChangeCueParams.RawMagnitude = ChangeNumber;
				OwningAbilitySystemComponent->ExecuteGameplayCue(ChangeCueTag, ChangeCueParams);
			}
		}

		// Clear the meta attribute that temporarily held damage
		SetDamage(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
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
			// 添加到全局委托的响应
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
			}

			// Calculate 'actual' damage applied that respects min and max health
			const float ChangeNumber = OldValue - NewValue;
			if (UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent())
			{
				// Broadcast a 'damage number' gameplay cue on the owning actor. Triggered on server, executes on all clients.
				const FGameplayTag ChangeCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.StaminaCostNumber"), /*ErrorIfNotFound=*/true);
				FGameplayCueParameters ChangeCueParams;
				ChangeCueParams.NormalizedMagnitude = 1.f;
				ChangeCueParams.RawMagnitude = ChangeValue;
				OwningAbilitySystemComponent->ExecuteGameplayCue(ChangeCueTag, ChangeCueParams);
			}
		}

		// Clear the meta attribute that temporarily held damage
		SetStaminaCost(0.0f);
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