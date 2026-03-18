#include <LabStatusAttributeSet.h>
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>
#include <GlobalDelegatesManager.h>
#include <SL_Macros.h>

ULabStatusAttributeSet::ULabStatusAttributeSet()
{

}

void ULabStatusAttributeSet::SetOwningActor(AActor* pOwnActor)
{
	RETURN_IF_TRUE(pOwnActor == nullptr);
	OwningActor = pOwnActor;
}

void ULabStatusAttributeSet::InitStatusAS_Implementation()
{
	// 由Unlua提供覆盖,C++中仅提供默认设置
	InitHealthAS(0.0f,100.0f);
}

void ULabStatusAttributeSet::InitHealthAS_Implementation(float MinValue, float MaxValue)
{
	InitHealth(MaxValue);
	InitMaxHealth(MaxValue);
	// 添加到全局委托的响应
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, MaxValue, MaxValue, MinValue, MaxValue);
	}
}

void ULabStatusAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	UE_LOG(LogTemp, Warning, TEXT("PreChange: Attribute '%s'"), *Attribute.AttributeName);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	Super::PreAttributeChange(Attribute, NewValue);
}

void ULabStatusAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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
		const float DamageValue = GetDamage();
		const float OldHealthValue = GetHealth();
		const float MinHealthValue = 0.0f;
		const float MaxHealthValue = GetMaxHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue - DamageValue, MinHealthValue, MaxHealthValue);

		if (OldHealthValue != NewHealthValue)
		{
			// Set the new health after clamping to min-max
			SetHealth(NewHealthValue);
			const float CurrentHealth = GetHealth();
			// 添加到全局委托的响应
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldHealthValue, CurrentHealth, MinHealthValue, MaxHealthValue);
			}

			// Calculate 'actual' damage applied that respects min and max health
			const float DamageNumber = OldHealthValue - NewHealthValue;
			if (UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent())
			{
				// Broadcast a 'damage number' gameplay cue on the owning actor. Triggered on server, executes on all clients.
				const FGameplayTag DamageCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.DamageNumber"), /*ErrorIfNotFound=*/true);
				FGameplayCueParameters DamageCueParams;
				DamageCueParams.NormalizedMagnitude = 1.f;
				DamageCueParams.RawMagnitude = DamageNumber;
				OwningAbilitySystemComponent->ExecuteGameplayCue(DamageCueTag, DamageCueParams);
			}
		}

		// Clear the meta attribute that temporarily held damage
		SetDamage(0.0f);
	}
}


void ULabStatusAttributeSet::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULabStatusAttributeSet, MaxHealth);
	DOREPLIFETIME(ULabStatusAttributeSet, Health);
}