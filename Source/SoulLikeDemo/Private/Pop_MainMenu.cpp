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

void UPop_MainMenu::InitMainMenu()
{
    // 创建 WrapBox
    MenuWrapBox = CreateWidget<UWrapBox>(this, UWrapBox::StaticClass());

    // 基本配置
    MenuWrapBox->SetOrientation(Orient_Horizontal);
    MenuWrapBox->SetWrapSize(500.0f);
    MenuWrapBox->SetInnerSlotPadding(FVector2D(10.0f, 10.0f));
    MenuWrapBox->SetHorizontalAlignment(HAlign_Center);

    ScrollBox->AddChild(MenuWrapBox);
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
            UWrapBoxSlot* menuSlot = MenuWrapBox->AddChildToWrapBox(newMenuItem);
             // 配置槽位属性
            if (menuSlot)
            {
                Slot->SetPadding(FMargin(5.0f));
                Slot->SetHorizontalAlignment(HAlign_Center);
                Slot->SetVerticalAlignment(VAlign_Center);
                //Slot->SetSize(FSlateChildSize(ESlateSizeRule::Auto)); // 自动大小
            }

            // 添加到队列
            m_arrButtonWidgets.Add(newMenuItem);
            newMenuItem->SetImageBrush(info.ButtonImg);
            newMenuItem->SetCenterTitle(info.ButtonText);
        }
    }
}

void UPop_MainMenu::RemoveMenuItem(const FName buttonFlag)
{

    TArray<UWidget*> Children = MenuWrapBox->GetAllChildren();
    for (UWidget* Child : Children)
    {
        if (UUI_MenuItem* menuItem = Cast<UUI_MenuItem>(Child))
        {
            if (menuItem->GetButtonFlag() == buttonFlag)
            {
                MenuWrapBox->RemoveChild(menuItem);
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
