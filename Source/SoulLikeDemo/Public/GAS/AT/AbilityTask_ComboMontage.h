#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include <SoulLikeGameGlobal.h>
#include "AbilityTask_ComboMontage.generated.h"

class UAnimMontage;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboMontageCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboMontageBlendOut);

/**
 * 连招专用蒙太奇播放Task
 *
 * 核心功能：
 * 1. 播放蒙太奇并监听AnimNotify事件
 * 2. 分离"逻辑窗口"和"物理混合时机"
 * 3. 输入预缓存：在AllowBlend之前按下输入会被暂存
 * 4. AllowBlend到达后自动消费暂存的输入
 */
UCLASS()
class SOULLIKEDEMO_API UAbilityTask_ComboMontage : public UAbilityTask
{
    GENERATED_BODY()

public:
    // 蒙太奇正常播完->通知GA结束
    UPROPERTY(BlueprintAssignable)
        FOnComboMontageCompleted OnCompleted;

    // 被下一招打断->通知GA结束
    UPROPERTY(BlueprintAssignable)
        FOnComboMontageBlendOut OnBlendOut;

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

    /** 当AllowBlend通知到达时调用 */
    void OnAllowBlendReached();

    /**
     * 当收到玩家输入时由ComboManager调用
     * @param InputAction 玩家按下的动作（轻攻击/重攻击等）
     * @return true=成功缓存或触发打断, false=不在接收窗口
     */
    UFUNCTION(BlueprintCallable, Category = "Combo")
        bool OnInputReceived(EComboInputActionType InputAction);

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

private:
    // 播放的蒙太奇
    UPROPERTY()
        UAnimMontage* Montage;

    // 混合出时间
    float BlendOutTime = 0.2f;

    // 播放速率
    float PlayRate = 1.0f;

    // ====== 核心状态标志 ======

    /** 是否已到达AllowBlend位置（允许物理打断） */
    bool bReadyToBlend = false;

    /** 在AllowBlend之前是否有暂存的输入 */
    bool bHasPendingInput = false;

    /** 暂存的输入Action */
    UPROPERTY()
    EComboInputActionType PendingInputAction;

    /** 是否已经结束（防止重复触发） */
    bool bHasFinished = false;
};