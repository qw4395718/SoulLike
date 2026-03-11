#include <LabHealthAttributeSet.h>
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>

ULabHealthAttributeSet::ULabHealthAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void ULabHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	UE_LOG(LogTemp, Warning, TEXT("PreChange: Attribute '%s'"), *Attribute.AttributeName);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	Super::PreAttributeChange(Attribute, NewValue);
}

void ULabHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	UE_LOG(LogTemp, Warning, TEXT("PostChange: Attribute -> %.2f"), Data.EvaluatedData.Magnitude);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float CurrentHealth = GetHealth();
		OnHealthChanged.Broadcast(this, CurrentHealth);
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Convert into -Health and then clamp
		const float DamageValue = GetDamage();
		const float OldHealthValue = GetHealth();
		const float MaxHealthValue = GetMaxHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue - DamageValue, 0.0f, MaxHealthValue);

		if (OldHealthValue != NewHealthValue)
		{
			// Set the new health after clamping to min-max
			SetHealth(NewHealthValue);
		}

		// Clear the meta attribute that temporarily held damage
		SetDamage(0.0f);
	}
}


void ULabHealthAttributeSet::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULabHealthAttributeSet, MaxHealth);
	DOREPLIFETIME(ULabHealthAttributeSet, Health);
}