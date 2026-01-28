// Pop_MainMenu.cpp
#include "Pop_MainMenu.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

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
     for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
       if(buttonInfo.ButtonTag == info.ButtonTag)
       {// 进行更新
            buttonInfo = info;
            return;
       }
    }
}

void UPop_MainMenu::UpdateInfoToUI()
{
    // 全量更新
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
            m_mapButtonWidgets.Add(info.ButtonTag,newMenuItem);
        }
    }
}

void UPop_MainMenu::RemoveMenuItem(const FName buttonFlag)
{

}

void UPop_MainMenu::UpdateMenuItem(const FMenuButtonInfo& info)
{
    
}

void UPop_MainMenu::ClearAllMenuItems()
{

}
