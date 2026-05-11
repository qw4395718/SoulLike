// Public/GAS/GA/SL_GameplayAbilityUseItem.h
#pragma once

#include "CoreMinimal.h"
#include "SL_GameplayAbilityBase.h"
#include "ItemDataStruct.h"
#include "SL_GameplayAbilityUseItem.generated.h"

/**
 * 通用道具使用能力
 * 通过事件传递的道具ID查表获取配置，动态应用效果
 * 整个游戏生命周期只需授予一次
 */
UCLASS()
class SOULLIKEDEMO_API USL_GameplayAbilityUseItem : public USL_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	USL_GameplayAbilityUseItem();

	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle InHandle,
		const FGameplayAbilityActorInfo* InActorInfo,
		const FGameplayAbilityActivationInfo InActivationInfo,
		const FGameplayEventData* InTriggerEventData) override;

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 从触发器事件数据中解析道具ID
	FName ParseItemIDFromEventData(const FGameplayEventData* InEventData) const;

	// 查表获取道具配置并应用效果
	void ApplyItemEffectByID(const FGameplayAbilityActorInfo* InActorInfo, FName InItemID);

	// 应用道具效果到自身和目标
	void ApplyItemEffects(const FGameplayAbilityActorInfo* InActorInfo, const FItemDataRow& InItemData);

	// 广播道具使用事件
	void BroadcastItemUsed(const FGameplayAbilityActorInfo* InActorInfo, FName InItemID);

	// 蒙太奇播放完成回调
	UFUNCTION()
		void OnUseMontageCompleted();

	// 蒙太奇播放中断回调
	UFUNCTION()
		void OnUseMontageInterrupted();

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 使用道具的蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
		UAnimMontage* UseItemMontage;

	// 缓存的AbilityActorInfo
	const FGameplayAbilityActorInfo* CachedActorInfo;

	// 当前使用的道具ID
	FName CurrentItemID;
};