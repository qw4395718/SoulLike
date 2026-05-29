// SL_UINavigationTypes.h
#pragma once

#include "CoreMinimal.h"
#include "SoulLikeGameGlobal.h"
#include "SL_UINavigationTypes.generated.h"

/************************************************************************/
/* 导航模式枚举                                                                   */
/************************************************************************/
UENUM(BlueprintType)
enum class EUINavigationMode : uint8
{
	// 压栈模式：将目标页面推入导航栈顶，栈深度+1
	Push		UMETA(DisplayName = "压栈"),
	// 替换模式：关闭当前栈顶，打开目标页面替换之，栈深度不变
	ReplaceTop	UMETA(DisplayName = "替换栈顶"),
	// 清栈模式：关闭导航栈中所有页面，清空栈，打开目标作为新的根页面
	ClearStack	UMETA(DisplayName = "清空栈后压入"),
};

/************************************************************************/
/* 导航载荷 - 页面间传参                                                          */
/************************************************************************/
USTRUCT(BlueprintType)
struct FUINavigationPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TMap<FName, int32> IntParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TMap<FName, float> FloatParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TMap<FName, FString> StringParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TMap<FName, UObject*> ObjectParams;

	bool IsEmpty() const
	{
		return IntParams.Num() == 0 && FloatParams.Num() == 0
			&& StringParams.Num() == 0 && ObjectParams.Num() == 0;
	}
};

/************************************************************************/
/* 导航上下文 - 传递给Widget的事件回调                                               */
/************************************************************************/
USTRUCT(BlueprintType)
struct FUINavigationContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		EWidgetType SourceType = EWidgetType::EWIDGET_None;

	UPROPERTY(BlueprintReadOnly)
		EWidgetType TargetType = EWidgetType::EWIDGET_None;

	UPROPERTY(BlueprintReadOnly)
		FUINavigationPayload Payload;

	UPROPERTY(BlueprintReadOnly)
		bool bIsBackNavigation = false;
};

/************************************************************************/
/* 导航栈条目（内部使用，不暴露给蓝图）                                                  */
/************************************************************************/
struct FUINavigationEntry
{
	EWidgetType WidgetType;
	TWeakObjectPtr<class UUserWidget> Widget;
	FUINavigationPayload Payload;
	double EnteredTime = 0.0;
};
