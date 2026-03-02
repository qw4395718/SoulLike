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
#include "Components/WrapBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "UIManagerSubsystem.h"

void UPop_MainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // 完成初始化
    
}

void UPop_MainMenu::InitMainMenu()
{
    RETURN_IF_TRUE(m_scrollBox == nullptr || m_menuWrapBox == nullptr);
    m_scrollBox->AddChild(m_menuWrapBox);
}

void UPop_MainMenu::OnMenuButtonClicked(const FName ButtonTag)
{
    UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
        for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
        {
            if(buttonInfo.ButtonTag == ButtonTag)
            {
                UIManager->OpenWidget(buttonInfo.linkWidgetIndex);
                return;
            }
        }

    }
}

void UPop_MainMenu::SetButtonInfos(const TArray<FMenuButtonInfo>& infos)
{
    ClearAllButtonInfos();
    for (int i = 0; i < infos.Num(); i++)
    {
        AddButtonInfo(infos[i]);
    }
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
            RemoveMenuItem(buttonFlag);
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
            UpdateMenuItem(buttonInfo);
            return;
       }
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
             // 添加到 WrapBox 并获取槽位
            UWrapBoxSlot* menuSlot = m_menuWrapBox->AddChildToWrapBox(newMenuItem);
             // 配置槽位属性
            if (menuSlot)
            {
                menuSlot->SetPadding(FMargin(5.0f));
                menuSlot->SetHorizontalAlignment(HAlign_Fill);
                menuSlot->SetVerticalAlignment(VAlign_Fill);
            }
         

            // 添加到队列
            m_mapButtonWidgets.Add(info.ButtonTag,newMenuItem);
            newMenuItem->SetImageBrush(info.ButtonImg);
            newMenuItem->SetCenterTitle(info.ButtonText);
   //         // 添加绑定函数
   //         const FName ButtonTag = info.ButtonTag;
   //         newMenuItem->OnClicked.AddLambda(this,[this,ButtonTag]()
			//{
			//	OnMenuButtonClicked(ButtonTag);
			//}
			//);
        }
    }
}

void UPop_MainMenu::RemoveMenuItem(const FName buttonFlag)
{
    TArray<UWidget*> Children = m_menuWrapBox->GetAllChildren();
    for (UWidget* Child : Children)
    {
        if (UUI_MenuItem* menuItem = Cast<UUI_MenuItem>(Child))
        {
            if (menuItem->GetButtonFlag() == buttonFlag)
            {
                m_menuWrapBox->RemoveChild(menuItem);
                break;
            }
        }
    }

    for (const FMenuButtonInfo& buttonInfo : m_arrButtonInfos)
    {
        if (buttonInfo.ButtonTag == buttonFlag)
        {// 已存在该信息则进行更新
            m_arrButtonInfos.Remove(buttonInfo);
            return;
        }
    }
}

void UPop_MainMenu::UpdateMenuItem(const FMenuButtonInfo& info)
{
    UUI_MenuItem** widgetpptr = m_mapButtonWidgets.Find(info.ButtonTag);
    if (widgetpptr)
    {
        // 更新数据
        (*widgetpptr)->SetImageBrush(info.ButtonImg);
        (*widgetpptr)->SetCenterTitle(info.ButtonText);
    }
}

void UPop_MainMenu::ClearAllMenuItems()
{
    m_mapButtonWidgets.Empty();
}
