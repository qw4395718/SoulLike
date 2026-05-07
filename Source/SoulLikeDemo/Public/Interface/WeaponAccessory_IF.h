// Public/Interface/WeaponAccessory_IF.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "WeaponAccessory_IF.generated.h"

class ASL_WeaponBase;  // 前向声明

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UWeaponAccessory_IF : public UInterface
{
    GENERATED_BODY()
};

/**
 * 武器访问接口
 * 提供给动画通知、行为树等获取角色当前持有的武器引用
 */
class SOULLIKEDEMO_API IWeaponAccessory_IF
{
    GENERATED_BODY()

public:
    // 获取左手武器引用（返回原始指针，因为调用方通常在同一个GameThread上）
    UFUNCTION(BlueprintCallable, Category = "WeaponAccessory")
        virtual ASL_WeaponBase* GetLeftHandWeapon() const = 0;

    // 获取右手武器引用
    UFUNCTION(BlueprintCallable, Category = "WeaponAccessory")
        virtual ASL_WeaponBase* GetRightHandWeapon() const = 0;

    // 获取指定手的武器（0=左手, 1=右手）
    UFUNCTION(BlueprintCallable, Category = "WeaponAccessory")
        virtual ASL_WeaponBase* GetWeaponByHand(int32 HandIndex) const = 0;
};