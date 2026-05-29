// SL_UserWidgetBase.cpp
#include "SL_UserWidgetBase.h"
#include "Manager/UIManagerSubsystem.h"

/************************************************************************/
/* 导航生命周期实现                                                                 */
/************************************************************************/
void USL_UserWidgetBase::OnNavigatedTo_Implementation(const FUINavigationContext& InContext)
{
	// 蓝图可重写
}

void USL_UserWidgetBase::OnNavigatedFrom_Implementation(const FUINavigationContext& InContext)
{
	// 蓝图可重写
}

void USL_UserWidgetBase::OnNavigatedBack_Implementation(const FUINavigationContext& InContext)
{
	// 默认行为：回退到 OnNavigatedTo
	OnNavigatedTo(InContext);
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/
EUIInputModeRequirement USL_UserWidgetBase::GetResolvedInputModeRequirement() const
{
	if (InputModeRequirement != EUIInputModeRequirement::Inherit)
	{
		return InputModeRequirement;
	}
	// Inherit → 从层级配置读取默认值
	return UILayerUtils::GetLayerConfig(Layer).DefaultInputMode;
}

void USL_UserWidgetBase::CloseSelf()
{
	if (WidgetType == EWidgetType::EWIDGET_None)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("USL_UserWidgetBase::CloseSelf - WidgetType not set, fallback to RemoveFromParent"));
		RemoveFromParent();
		return;
	}

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->CloseScreenWidget(WidgetType);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("USL_UserWidgetBase::CloseSelf - UIManagerSubsystem not found, fallback to RemoveFromParent"));
		RemoveFromParent();
	}
}
