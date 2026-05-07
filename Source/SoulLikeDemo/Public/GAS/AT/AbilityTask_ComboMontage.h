#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include <SoulLikeGameGlobal.h>
#include "AbilityTask_ComboMontage.generated.h"

class UAnimMontage;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboMontageCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboMontageInterrupted);

/** OnInputReceived 的返回值状态 */
UENUM()
enum class EComboInputHandledResult : uint8
{
	Accepted,           // 接受输入，等待 BlendOut
	AcceptedAndBlended, // 接受输入，且已经触发了 BlendOut（绑定已不需要等待）
	Rejected,           // 不接受
};

/**
 * 连招专用蒙太奇播放Task
 *
 * 职责：仅管理动画播放和混合时机
 * - 不参与业务逻辑（不持有连招表、不判断输入）
 * - 对外暴露 IsReadyToBlend() 查询状态
 * - 对外暴露 RequestBlendOut() 主动打断
 */

UCLASS()
class SOULLIKEDEMO_API UAbilityTask_ComboMontage : public UAbilityTask
{
    GENERATED_BODY()

public:
	/****************************************************************************/
    /*										外部调用									*/
    /****************************************************************************/
    /**
     * 创建Task的静态工厂方法
     * @param OwningAbility    拥有的GA
     * @param MontageToPlay    要播放的蒙太奇
     * @param InBlendOutTime   被打断时的混合时间
     * @param InPlayRate       播放速率
     */
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
        meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
        static UAbilityTask_ComboMontage* CreateComboMontageTask(
            UGameplayAbility* OwningAbility,
            UAnimMontage* MontageToPlay,
            float InBlendOutTime = 0.2f,
            float InPlayRate = 1.0f
        );

    // ========== 动画通知回调接口 ==========
	/** 当前是否已到达AllowBlend位置 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
		bool IsReadyToBlend() const { return bReadyToBlend; }

	/** 被动画通知调用：标记已到达允许混合的位置 */
	void OnAllowBlendReached(FGameplayTag CurrentWindowTag);

	UFUNCTION(BlueprintCallable, Category = "Combo")
        EComboInputHandledResult OnInputReceived(EComboInputActionType InputAction);



	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
    // 蒙太奇相关回调
    UFUNCTION()
        void OnMontageCompleted(UAnimMontage* InMontage, bool bInterrupted);

    UFUNCTION()
        void OnMontageBlendOut(UAnimMontage* InMontage, bool bInterrupted);

    UFUNCTION()
        void OnMontageInterrupted(UGameplayAbility* ActiveAbility);

    // 执行打断混合
    void ExecuteBlendOut();

    void RemoveAllDelegates();

public:
	/****************************************************************************/
	/*										外部调用									*/
	/****************************************************************************/

	// 蒙太奇正常播完->通知GA结束
	UPROPERTY(BlueprintAssignable)
		FOnComboMontageCompleted OnCompleted;

	/** 被连招打断时触发（BlendOut完成后） */
	UPROPERTY(BlueprintAssignable)
		FOnComboMontageInterrupted OnInterrupted;

private:
    // 播放的蒙太奇
    UPROPERTY()
        UAnimMontage* Montage;

    UPROPERTY()
        FGameplayTag ComboWindowTag;

	/** 在AllowBlend之前是否有暂存的输入 */
	bool bHasPendingInput = false;

    // 混合出时间
    float BlendOutTime = 0.2f;

    // 播放速率
    float PlayRate = 1.0f;

    /** 是否已到达AllowBlend位置（允许物理打断） */
    bool bReadyToBlend = false;

    /** 在AllowBlend之前是否有暂存的输入 */
    bool bBlendOutRequested = false;

    /** 是否已经结束（防止重复触发） */
    bool bHasFinished = false;
};