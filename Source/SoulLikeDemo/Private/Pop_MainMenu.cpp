// Pop_MainMenu.cpp
#include "Pop_MainMenu.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "UI_MenuItem.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

void UPop_MainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // 完成初始化
    
}

void UPop_MainMenu::OnMenuButtonClicked(FName ButtonTag)
{
    
}

void UPop_MainMenu::SetButtonInfos(const TArray<FMenuButtonInfo>& infos)
{
    ClearAllButtonInfos();
    m_arrButtonInfos = infos;
}

void UPop_MainMenu::ClearAllButtonInfos()
{
    m_arrButtonInfos.Reset();
}

void UPop_MainMenu::AddButtonInfo(const FMenuButtonInfo& info)
{
    for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
       if(buttonInfo.ButtonTag == info.ButtonTag)
       {// 已存在该信息则进行更新
            UpdateButtonInfo(info);
            // 更新按钮
            UpdateMenuItem(info);
            return;
       }
    }

    m_arrButtonInfos.Add(info);
    CreateNewMenuItem(info);
}

void UPop_MainMenu::RemoveButtonInfo(const FName buttonFlag)
{
   for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
       if(buttonInfo.ButtonTag == buttonFlag)
       {// 已存在该信息则进行更新
            m_arrButtonInfos.Remove(buttonInfo);
            return;
       }
    }
}

void UPop_MainMenu::UpdateButtonInfo(const FMenuButtonInfo& info)
{
    for (FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
       if(buttonInfo.ButtonTag == info.ButtonTag)
       {// 进行更新
            buttonInfo.ButtonText = info.ButtonText;
            buttonInfo.ButtonImg = info.ButtonImg;
            buttonInfo.ButtonTag = info.ButtonTag;
            buttonInfo.linkWidgetIndex = info.linkWidgetIndex;
            return;
       }
    }
}

void UPop_MainMenu::RefreshMenuToUI()
{
	if (!GridPanel) return;
    // 清理现有的按钮
    GridPanel->ClearChildren();

	// 计算总行数
	int32 TotalRows = FMath::CeilToInt((float)m_arrButtonWidgets.Num() / ColumnsPerRow);

	// 设置Grid的行列数
	SetupGridLayout();

	// 创建按钮
	for (int32 Index = 0; Index < m_arrButtonWidgets.Num(); Index++)
	{
		int32 Row = Index / ColumnsPerRow;
		int32 Column = Index % ColumnsPerRow;

		// 创建按钮控件
        UUI_MenuItem* ButtonWidget = m_arrButtonWidgets[Index];
		if (ButtonWidget)
		{
			// 添加到Grid
			UGridSlot* GridSlot = GridPanel->AddChildToGrid(ButtonWidget, Row, Column);

			if (GridSlot)
			{
				// 设置填充和对齐
				GridSlot->SetPadding(ButtonPadding);
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);

				// 设置大小
				GridSlot->SetColumnSpan(1);
				GridSlot->SetRowSpan(1);
			}
		}
	}

}

void UPop_MainMenu::SetupGridLayout()
{
	if (!GridPanel) return;

	int32 TotalRows = FMath::CeilToInt((float)m_arrButtonWidgets.Num() / ColumnsPerRow);

	// 清除现有行列设置
	//GridPanel->ClearRowFill();
	//GridPanel->ClearColumnFill();

	// 设置列填充系数（等宽）
	for (int32 Col = 0; Col < ColumnsPerRow; Col++)
	{
		GridPanel->SetColumnFill(Col, 1.0f);
	}

	// 设置行填充系数（等高）
	for (int32 Row = 0; Row < TotalRows; Row++)
	{
		GridPanel->SetRowFill(Row, 1.0f);
	}
}

void UPop_MainMenu::CreateNewMenuItem(const FMenuButtonInfo& info)
{
    // 创建新菜单按钮
	if (m_menuItemWidgetClass && GEngine->GameViewport)
	{
        UUI_MenuItem* newMenuItem = CreateWidget<UUI_MenuItem>(
			GetWorld(),
			m_menuItemWidgetClass
			);

        if (newMenuItem)
		{
            // 添加到队列
            m_arrButtonWidgets.Add(newMenuItem);
            newMenuItem->SetImageBrush(info.ButtonImg);
            newMenuItem->SetCenterTitle(info.ButtonText);
        }
    }
}

void UPop_MainMenu::RemoveMenuItem(const FName buttonFlag)
{
    for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
        if (buttonInfo.ButtonTag == buttonFlag)
        {// 已存在该信息则进行更新
            m_arrButtonInfos.Remove(buttonInfo);
            return;
        }
    }
    m_arrButtonWidgets.;
}

void UPop_MainMenu::UpdateMenuItem(const FMenuButtonInfo& info)
{
    UUI_MenuItem** widgetpptr = m_arrButtonWidgets.Find(info.ButtonTag);
    if (widgetpptr)
    {
        // 更新数据
        (*widgetpptr)->SetImageBrush(info.ButtonImg);
        (*widgetpptr)->SetCenterTitle(info.ButtonText);
    }
}

void UPop_MainMenu::ClearAllMenuItems()
{
    m_arrButtonWidgets.Empty();
}
