#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_ListItemBase.h"
#include "UI_MenuItem.generated.h"

class UImage;

/**
 * 菜单功能项
 *
 * 继承自 UUI_ListItemBase，在通用列表项基础上增加新获取提示标记。
 * 蓝图中应外套 SizeBox 以控制固定尺寸。
 */
UCLASS()
class SOULLIKEDEMO_API UUI_MenuItem : public UUI_ListItemBase
{
	GENERATED_BODY()

public:
	UUI_MenuItem(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
	void SetNewAcquireImageVisible(bool visible);

	UFUNCTION()
	FName GetButtonFlag();

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 新获取提示图标
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* m_newTipImage;

	// 在容器标识
	FName m_menuFuncName;
};
