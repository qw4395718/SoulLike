// Private/Component/Character/SL_StaminaComponent.cpp

#include "SL_StaminaComponent.h"
#include "AbilitySystemComponent.h"
#include "SL_StatusAttributeSet.h"
#include "GlobalDelegatesManager.h"
#include <AbilitySystemInterface.h>

USL_StaminaComponent::USL_StaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentRegenState = EStaminaRegenState::Regenerating;
	bWasOverdraw = false;
}

// ==================== 初始化 ====================

void USL_StaminaComponent::InitializeStaminaComponent()
{
	// ===== 创建三个GameplayEffect =====
	if (!CostEffectClass)
	{
		/*CostEffectClass = CreateCostEffect()->GetClass();*/
		CostEffectClass = LoadClass<UGameplayEffect>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/GAS/GE/Player/GE_StaminaCost.GE_StaminaCost_C")
			);
	}

	if (!RegenEffectClass)
	{
		/*RegenEffectClass = CreateRegenEffect()->GetClass();*/
		RegenEffectClass = LoadClass<UGameplayEffect>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/GAS/GE/Player/GE_StaminaRegen.GE_StaminaRegen_C")
			);
	}

	if (!DelayEffectClass)
	{
		/*DelayEffectClass = CreateDelayEffect()->GetClass();*/
		DelayEffectClass = LoadClass<UGameplayEffect>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/GAS/GE/Player/GE_StaminaDelayRegen.GE_StaminaDelayRegen_C")
			);
	}


	// 默认启动恢复
	CurrentRegenState = EStaminaRegenState::Regenerating;
	ActivateRegen();
}

// ==================== ComboManager 接口 ====================

bool USL_StaminaComponent::CanAffordCost(float CostAmount) const
{
	if (!GetCacheASC()) return false;

	const USL_StatusAttributeSet* AttrSet = CachedASC->GetSet<USL_StatusAttributeSet>();
	if (!AttrSet) return false;

	// 体力大于0即可（即使不足以支付全部消耗，也可以透支）
	return AttrSet->GetStamina() > 0.0f;
}

bool USL_StaminaComponent::IsOverdrawAttack(float CostAmount) const
{
	if (!GetCacheASC()) return false;

	const USL_StatusAttributeSet* AttrSet = CachedASC->GetSet<USL_StatusAttributeSet>();
	if (!AttrSet) return false;

	float CurrentStamina = AttrSet->GetStamina();
	return (CurrentStamina > 0.0f && CurrentStamina < CostAmount);
}

float USL_StaminaComponent::GetCurrentStamina() const
{
	if (!GetCacheASC()) return 0.0f;
	const USL_StatusAttributeSet* AttrSet = CachedASC->GetSet<USL_StatusAttributeSet>();
	return AttrSet ? AttrSet->GetStamina() : 0.0f;
}

float USL_StaminaComponent::GetMaxStamina() const
{
	if (!GetCacheASC()) return 0.0f;
	const USL_StatusAttributeSet* AttrSet = CachedASC->GetSet<USL_StatusAttributeSet>();
	return AttrSet ? AttrSet->GetMaxStamina() : 0.0f;
}

float USL_StaminaComponent::GetStaminaPercentage() const
{
	return GetMaxStamina() > 0.0f ? (GetCurrentStamina() / GetMaxStamina()) : 0.0f;
}

void USL_StaminaComponent::ConsumeStamina(float InAmount)
{
	if (!GetCacheASC() || InAmount <= 0.0f) return;

	// 通过GE方式消耗（走GAS管道，支持网络同步）
	FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();

	if (CostEffectClass)
	{
		FGameplayEffectSpecHandle Spec = CachedASC->MakeOutgoingSpec(CostEffectClass, 1.0f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaCostNumber")),
				InAmount
			);
			CachedASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}
}

// ==================== 连击生命周期 ====================

void USL_StaminaComponent::OnComboStarted()
{
	// 暂停恢复
	if (CurrentRegenState == EStaminaRegenState::Regenerating)
	{
		DeactivateRegen();
	}

	// 如果还有延迟恢复在运行，也取消
	if (DelayEffectHandle.IsValid() && CachedASC.IsValid())
	{
		CachedASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
		CachedASC->RemoveActiveGameplayEffect(DelayEffectHandle);
		DelayEffectHandle.Invalidate();
	}

	SetRegenState(EStaminaRegenState::Paused);
}

void USL_StaminaComponent::OnComboEnded()
{
	if (!GetCacheASC()) return;

	const USL_StatusAttributeSet* AttrSet = CachedASC->GetSet<USL_StatusAttributeSet>();
	if (!AttrSet) return;

	float CurrentStamina = AttrSet->GetStamina();

	if (CurrentStamina <= 0.0f || bWasOverdraw)
	{
		// 体力清空或透支 → 延迟恢复
		SetRegenState(EStaminaRegenState::Delayed);
		StartDelayRecovery();
		bWasOverdraw = false;
	}
	else
	{
		// 体力充足 → 立即恢复
		SetRegenState(EStaminaRegenState::Regenerating);
		ActivateRegen();
	}
}

// ==================== 配置接口 ====================

void USL_StaminaComponent::SetRegenEffect(TSubclassOf<UGameplayEffect> InEffectClass)
{
	RegenEffectClass = InEffectClass;
}

void USL_StaminaComponent::SetDelayEffect(TSubclassOf<UGameplayEffect> InEffectClass)
{
	DelayEffectClass = InEffectClass;
}

void USL_StaminaComponent::SetCostEffect(TSubclassOf<UGameplayEffect> InEffectClass)
{
	CostEffectClass = InEffectClass;
}

// ==================== 恢复控制 ====================

void USL_StaminaComponent::ActivateRegen()
{
	if (!GetCacheASC() || !RegenEffectClass) return;
	if (RegenEffectHandle.IsValid()) return;  // 已经激活

	FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = CachedASC->MakeOutgoingSpec(RegenEffectClass, 1.0f, Context);
	if (Spec.IsValid())
	{
		// ===== 设置恢复速率（每秒恢复15点，每0.15秒恢复15*0.15=2.25点） =====
		Spec.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaRegenNumber")),
			2.25f  // 每tick恢复2.25体力（每秒15点）
		);

		RegenEffectHandle = CachedASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		UE_LOG(LogTemp, Verbose, TEXT("Stamina regen activated"));
	}
}

void USL_StaminaComponent::DeactivateRegen()
{
	if (!GetCacheASC()) return;

	if (RegenEffectHandle.IsValid())
	{
		CachedASC->RemoveActiveGameplayEffect(RegenEffectHandle);
		RegenEffectHandle.Invalidate();
		UE_LOG(LogTemp, Verbose, TEXT("Stamina regen deactivated"));
	}
}

void USL_StaminaComponent::StartDelayRecovery()
{
	if (!GetCacheASC() || !DelayEffectClass) return;

	FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = CachedASC->MakeOutgoingSpec(DelayEffectClass, 1.0f, Context);
	if (Spec.IsValid())
	{
		// 注册延迟GE移除的回调
		CachedASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &USL_StaminaComponent::OnDelayEffectRemoved);

		DelayEffectHandle = CachedASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		UE_LOG(LogTemp, Verbose, TEXT("Stamina delay recovery started"));
	}
}

void USL_StaminaComponent::OnDelayEffectRemoved(const FActiveGameplayEffect& Effect)
{
	if (Effect.Handle == DelayEffectHandle)
	{
		DelayEffectHandle.Invalidate();
		CachedASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);

		// 延迟结束，恢复恢复
		SetRegenState(EStaminaRegenState::Regenerating);
		ActivateRegen();

		UE_LOG(LogTemp, Verbose, TEXT("Stamina delay finished, regen resumed"));
	}
}

// ==================== 状态管理 ====================

void USL_StaminaComponent::SetRegenState(EStaminaRegenState NewState)
{
	if (CurrentRegenState == NewState) return;

	EStaminaRegenState OldState = CurrentRegenState;
	CurrentRegenState = NewState;

	UE_LOG(LogTemp, Verbose, TEXT("Stamina regen state: %d -> %d"), (int32)OldState, (int32)NewState);
}


// ==================== 创建三个GameplayEffect ====================

/**
 * 创建体力消耗GE（Instant类型）
 * 作用：通过 StaminaCost Meta属性消耗体力
 */
UGameplayEffect* USL_StaminaComponent::CreateCostEffect()
{
	// 创建一个新的GameplayEffect对象
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("StaminaCostEffect"));
	if (!Effect) return nullptr;

	// 设置Duration Policy为Instant（立即生效）
	Effect->DurationPolicy = EGameplayEffectDurationType::Instant;

	// 创建一个Modifier
	FGameplayModifierInfo& Modifier = Effect->Modifiers.AddDefaulted_GetRef();

	// 设置Modifier的属性为 StaminaCost（Meta属性）
	Modifier.Attribute = USL_StatusAttributeSet::GetStaminaCostAttribute();

	// 设置操作类型为Add（增加消耗值，PostGameplayEffectExecute会处理减法）
	Modifier.ModifierOp = EGameplayModOp::Additive;

	// 使用SetByCaller来动态设置数值
	FSetByCallerFloat temp;
	temp.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaRegenNumber"));
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(
		temp
	);

	return Effect;
}

/**
 * 创建体力恢复GE（Infinite + Periodic）
 * 作用：每0.15秒执行一次，通过 StaminaRegen Meta属性恢复体力
 */
UGameplayEffect* USL_StaminaComponent::CreateRegenEffect()
{
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("StaminaRegenEffect"));
	if (!Effect) return nullptr;

	// 设置为Infinite（无限持续，直到被移除）
	Effect->DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 设置为周期性执行：每0.15秒执行一次
	Effect->PeriodicInhibitionPolicy = EGameplayEffectPeriodInhibitionRemovedPolicy::ExecuteAndResetPeriod;
	Effect->Period.Value = 0.15f;  // 执行间隔
	Effect->bExecutePeriodicEffectOnApplication = false; // 不立即执行第一次

	// 创建一个Modifier
	FGameplayModifierInfo& Modifier = Effect->Modifiers.AddDefaulted_GetRef();

	// 设置Modifier的属性为 StaminaRegen（Meta属性）
	Modifier.Attribute = USL_StatusAttributeSet::GetStaminaRegenAttribute();

	// 设置操作类型为Add（增加恢复值，PostGameplayEffectExecute会处理加法）
	Modifier.ModifierOp = EGameplayModOp::Additive;

	// 使用SetByCaller来动态设置恢复速率
	FSetByCallerFloat temp;
	temp.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaRegenNumber"));
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(
		temp
	);

	return Effect;
}

/**
 * 创建延迟恢复GE（Duration类型）
 * 作用：持续1.5秒，结束后触发OnDelayEffectRemoved回调
 */
UGameplayEffect* USL_StaminaComponent::CreateDelayEffect()
{
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("StaminaDelayEffect"));
	if (!Effect) return nullptr;

	// 设置为Has Duration（有持续时间）
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 设置持续时间为1.5秒
	Effect->DurationMagnitude = FGameplayEffectModifierMagnitude(
		FScalableFloat(1.5f)
	);

	// 不执行Periodic（不需要周期执行）
	Effect->PeriodicInhibitionPolicy = EGameplayEffectPeriodInhibitionRemovedPolicy::NeverReset;

	return Effect;
}

UAbilitySystemComponent* USL_StaminaComponent::GetCacheASC() const
{
	// 使用缓存
	if (CachedASC.IsValid())
	{
		return CachedASC.Get();
	}

	// 从当前控制的 Pawn 上查找
	if (IAbilitySystemInterface* ASC_IF = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		// 从接口获取目标的 AbilitySystemComponent
		if (UAbilitySystemComponent* ASC = ASC_IF->GetAbilitySystemComponent())
		{
			CachedASC = ASC;
			return CachedASC.Get();
		}
	}

	return nullptr;
}
