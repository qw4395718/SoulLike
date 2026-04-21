#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SL_GameplayAbilityComboBase.generated.h"

UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityComboBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USL_GameplayAbilityComboBase();

	// 技能对应的Lua文件路径
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lua")
		FString LuaFilePath;

	// 技能配置数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float ManaCost = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownDuration = 2.0f;

	//// 事件委托，用于通知Lua
	//UPROPERTY(BlueprintAssignable, Category = "Events")
	//	FOnAbilityActivatedDelegate OnAbilityActivated;

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

	// 供Lua调用的接口
	UFUNCTION(BlueprintCallable, Category = "Ability")
		void PlayMontageForAbility(UAnimMontage* Montage, const FGameplayAbilityActivationInfo ActivationInfo, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Ability")
		void ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, const AActor* const Target, float Level = 1.0f);

protected:
	UFUNCTION()
		void OnMontageCompleted();

	UFUNCTION()
		void OnMontageInterrupted();

	FGameplayAbilitySpecHandle CurrentHandle;

	const FGameplayAbilityActorInfo* CurrentActorInfo;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> AcitvateEffectClass;

	UPROPERTY()
		float AcitvateEffectLevel;
};