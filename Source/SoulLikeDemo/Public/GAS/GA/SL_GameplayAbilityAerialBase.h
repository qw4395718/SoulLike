#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SoulLikeGameGlobal.h"
#include "SL_GameplayAbilityAerialBase.generated.h"

class UAbilityTask_Aerial;
class UAnimMontage;

/**
 * 浮空技 GA 基类
 *
 * 管理浮空技三段生命周期：
 *   升空(Launch) → 悬空循环(AirLoop) → 下劈/坠落(Dive) → 落地(Landed)
 *
 * 子类在蓝图中配置三段动画蒙太奇。
 */
UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityAerialBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USL_GameplayAbilityAerialBase();

	/************************************************************************/
	/*                          外部调用                                     */
	/************************************************************************/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** 外部通知：连招管理器收到下劈输入 */
	UFUNCTION(BlueprintCallable, Category = "Aerial")
	void RequestDive();

protected:
	/************************************************************************/
	/*                          蓝图事件                                     */
	/************************************************************************/
	/** 进入空中循环时调用（子类在此应用持续伤害 GE） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Aerial")
	void OnAirLoopStarted();

	/** 下劈开始时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Aerial")
	void OnDiveStarted();

	/** 落地完成时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Aerial")
	void OnAerialEnded();

	/** 升空蒙太奇播放完毕 */
	UFUNCTION()
	void OnLaunchMontageComplete();

	/** 进入空中循环 */
	UFUNCTION()
	void OnEnterAirLoop();

	/** 下劈开始 */
	UFUNCTION()
	void OnEnterDive();

	/** 浮空结束 */
	UFUNCTION()
	void OnAerialFinished();

protected:
	/************************************************************************/
	/*                        ComboInfo 驱动                                 */
	/************************************************************************/
	/** 从 ComboInfo 和 WeaponAnimSet 解析蒙太奇后创建 Task */
	void StartAerialTaskFromComboInfo();

private:
	UPROPERTY()
	UAbilityTask_Aerial* AerialTask;
};
