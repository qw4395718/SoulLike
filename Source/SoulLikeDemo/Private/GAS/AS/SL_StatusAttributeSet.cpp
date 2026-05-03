#include <SL_StatusAttributeSet.h>
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>
#include <GlobalDelegatesManager.h>
#include <SL_Macros.h>
#include <SL_CharacterBase.h>
#include <Components/CapsuleComponent.h>

USL_StatusAttributeSet::USL_StatusAttributeSet() 
{
	// 复活委托绑定
	if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
	{
		globalDelegatesManager->OnCharacterRevived.AddUObject(this,&USL_StatusAttributeSet::OnCharacterReLive);
	}
}

void USL_StatusAttributeSet::SetOwningActor(AActor* pOwnActor)
{
	RETURN_IF_TRUE(pOwnActor == nullptr);
	OwningActor = pOwnActor;
}

void USL_StatusAttributeSet::InitStatusAS_Implementation()
{
	InitHealthAS(0.0f,100.0f);
	InitStaminaAS(0.0f, 100.0f);
}

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
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeHealthChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
				// 血量归零
				if (CurrentValue == MinValue) {OnCharacterDeath();}
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
	else if (Data.EvaluatedData.Attribute == GetStaminaCostAttribute())
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
			if (UGlobalDelegatesManager* globalDelegatesManager = UGlobalDelegatesManager::Get(this))
			{
				globalDelegatesManager->OnAttributeStaminaChanged.Broadcast(OwningActor, OldValue, CurrentValue, MinValue, MaxValue);
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
void USL_StatusAttributeSet::OnCharacterDeath()
{
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    if (!ASC) return;

    AActor* OwnerActor = ASC->GetOwnerActor();
    if (!OwnerActor) return;

    // 1. 更新GAS标签
    ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Alive"));
    ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));

    // 2. 广播死亡事件
    if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
    {
        DelegateMgr->OnCharacterDied.Broadcast(OwnerActor, nullptr);
    }

    // 3. 禁用角色
    if (ACharacter* Char = Cast<ACharacter>(OwnerActor))
    {
        // 注意：UE4.26中GetMesh()可能返回null，需检查
        Char->SetActorEnableCollision(false);
        
        if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
        {
            PC->SetCinematicMode(true, false, false);
        }
    }

	// 4.添加布娃娃系统检验-后续需要拆分到动画模块中
	if (ACharacter* Char = Cast<ACharacter>(OwnerActor))
	{
		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (Char->GetMesh())
		{
			Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Char->GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			Char->GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"),true,true);
		}
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

	// 2.移除布娃娃系统,恢复正常的碰撞
	if (ACharacter* Char = Cast<ACharacter>(ReviveActor))
	{
		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		if (Char->GetMesh())
		{
			Char->GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Char->GetMesh()->SetAllBodiesSimulatePhysics(false);
		}
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