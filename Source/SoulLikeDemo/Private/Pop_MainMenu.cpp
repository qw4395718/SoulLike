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
    //if (m_arrButtonInfos.Find(info))
    //{
    //    // 已存在该信息
    //    return;
    //}

    

}

void UPop_MainMenu::RemoveButtonInfo(const FName buttonFlag)
{

}

void UPop_MainMenu::UpdateButtonInfo(const FMenuButtonInfo& info)
{

}

void UPop_MainMenu::CreateNewMenuItem(const FMenuButtonInfo& info)
{

}

void UPop_MainMenu::RemoveMenuItem(const FName buttonFlag)
{

}

void UPop_MainMenu::ClearAllMenuItems()
{

}
