#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SL_GameplayAbilityNPCBase.generated.h"

// ===== 新增：蒙太奇完成委托 =====
// 用于通知BTTask蒙太奇播放结束
DECLARE_MULTICAST_DELEGATE(FOnMontageCompletedDelegate);

// ===== 新增：蒙太奇状态枚举 =====
UENUM(BlueprintType)
enum class EMontagePlayState : uint8
{
    None,
    Playing,
    Completed,
    Interrupted
};

UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityNPCBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    USL_GameplayAbilityNPCBase();

    // ===== 原有字段 =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lua")
        FString LuaFilePath;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
        float ManaCost = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
        float CooldownDuration = 2.0f;

    // ===== 新增：激活此能力所需的GameplayTag =====
    // 行为树通过这个Tag来激活能力
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
        FGameplayTag ActivationTag;

    // ===== 新增：委托（供BTTask监听）=====
    FOnMontageCompletedDelegate OnMontageCompletedDelegate;

    // ===== 新增：获取当前蒙太奇播放状态 =====
    UFUNCTION(BlueprintPure, Category = "Ability")
        EMontagePlayState GetMontagePlayState() const { return MontagePlayState; }

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    // ===== 原有：结束能力 =====
    UFUNCTION(BlueprintCallable, Category = "Ability")
        void EndAbilityForBP(const FGameplayAbilitySpecHandle Handle,
            const FGameplayAbilityActorInfo& ActorInfo,
            const FGameplayAbilityActivationInfo ActivationInfo,
            bool bReplicateEndAbility,
            bool bWasCancelled);

    // ===== 修改：PlayMontageForAbility 添加完成回调 =====
    UFUNCTION(BlueprintCallable, Category = "Ability")
        void PlayMontageForAbility(UAnimMontage* Montage, 
            const FGameplayAbilityActivationInfo ActivationInfo, 
            float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Ability")
        void ApplyEffectToTarget(TSubclassOf<UGameplayEffect> GameEffectClass,
            AActor* Target, float Level = 1.0f);

public:
	// 蓝图可配置的效果类
	UPROPERTY(EditDefaultsOnly, Category = "AbilityGE|Config")
		TSubclassOf<UGameplayEffect> EffectClass;

	/** 这一招的动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Combo|Animation")
		UAnimMontage* AttackMontage;

protected:
    // ===== 修改：蒙太奇回调 =====
    UFUNCTION()
        void OnMontageCompleted();

    UFUNCTION()
        void OnMontageInterrupted();

    // ===== 新增：蒙太奇播放结束广播 =====
    void BroadcastMontageCompleted();

    FGameplayAbilitySpecHandle CurrentHandle;
    const FGameplayAbilityActorInfo* CurrentActorInfo;

    // ===== 新增：内部状态 =====
    EMontagePlayState MontagePlayState = EMontagePlayState::None;
};