#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include <Animation/AnimMontage.h>
#include "SL_GameplayAbilityComboBase.generated.h"

class UAbilityTask_ComboMontage;

UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityComboBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USL_GameplayAbilityComboBase();

/************************************************************************/
/*                              外部调用                                        */
/************************************************************************/
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnAbilityActivatedForLua(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo& ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo);

	// 重载GA的Activate响应函数
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void EndAbilityForBP(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo& ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled);

	UFUNCTION(BlueprintCallable, Category = "Ability")
		void ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, const AActor* const Target, float Level = 1.0f);

protected:

	/************************************************************************/
	/*                              内部调用                                        */
	/************************************************************************/
	UFUNCTION()
		void OnMontageCompleted();

	UFUNCTION()
		void OnMontageInterrupted();

	UFUNCTION()
		void OnComboInputReceived(FGameplayEventData EventData);

	UFUNCTION()
		void OnAllowBlendReceived(FGameplayEventData EventData);

	/** 执行缓存的连击 */
	void ExecutePendingCombo();

protected:
	/************************************************************************/
	/*                              可访问                                        */
	/************************************************************************/

	FGameplayAbilitySpecHandle CurrentHandle;

	const FGameplayAbilityActorInfo* CurrentActorInfo;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> AcitvateEffectClass;

	UPROPERTY()
		float AcitvateEffectLevel;

	/************************************************************************/
	/*                              蓝图配置                                       */
	/************************************************************************/
	/** 这一招的动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Combo|Animation")
		UAnimMontage* AttackMontage;

	/** 被下一招打断时的混合时间 */
	UPROPERTY(EditDefaultsOnly, Category = "Combo|Animation")
		float BlendOutTime = 0.15f;

	/** 是否需要等待AllowBlend才能被打断 */
	UPROPERTY(EditDefaultsOnly, Category = "Combo|Animation")
		bool bRespectBlendWindow = true;

	// 技能对应的Lua文件路径
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lua")
		FString LuaFilePath;

	// 技能配置数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float ManaCost = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownDuration = 2.0f;

private:
	/************************************************************************/
	/*                              内部参数                                        */
	/************************************************************************/
	UPROPERTY()
		UAbilityTask_ComboMontage* ComboMontageTask;

	// 输入缓存
	bool bHasPendingCombo = false;
	TSubclassOf<UGameplayAbility> PendingNextGA;

	// 标志
	bool bAllowBlendReached = false;
	bool bComboInputReceived = false;
};