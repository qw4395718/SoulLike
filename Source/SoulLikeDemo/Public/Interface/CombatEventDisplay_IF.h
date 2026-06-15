// Public/Interface/CombatEventDisplay_IF.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatEventDisplay_IF.generated.h"

struct FDamageFloatingTextData;

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UCombatEventDisplay_IF : public UInterface
{
    GENERATED_BODY()
};

/**
 * 战斗事件展示接口（事件通知型）
 * 提供伤害飘字、死亡事件等战斗相关的事件通知能力
 * 任何需要展示战斗事件的角色（玩家、敌人、Boss、召唤物等）都应实现此接口
 */
class SOULLIKEDEMO_API ICombatEventDisplay_IF
{
    GENERATED_BODY()

public:
    // 广播伤害飘字
    virtual void BroadcastDamageFloatingText(const struct FDamageFloatingTextData& InData) = 0;

    // 广播死亡事件(暂时不会使用-死亡由属性AS触发广播)
    virtual void BroadcastCharacterDeath(AActor* InDeadActor, AActor* InInstigator) = 0;
};
