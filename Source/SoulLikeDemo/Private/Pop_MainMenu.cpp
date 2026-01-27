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

UButton* UPop_MainMenu::AddMenuButton(const FMenuButtonInfo& ButtonInfo)
{
    // 检查重复标签
    if (m_mapButtonWidgets.Contains(ButtonInfo.ButtonTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("Button with tag %s already exists!"), *ButtonInfo.ButtonTag.ToString());
        return nullptr;
    }
    
    // 创建按钮控件
    UButton* NewButton = CreateWidget<UButton>(GetWorld(),UButton::StaticClass());
    if (!NewButton) return nullptr;
    
    // 创建文本控件并添加到按钮
    UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    ButtonText->SetText(ButtonInfo.ButtonText);
    
    // 设置文本样式
    ButtonText->SetJustification(ETextJustify::Center);
    
    // 将文本添加到按钮
    NewButton->AddChild(ButtonText);
    
    // 设置按钮点击事件
    NewButton->OnClicked.AddDynamic(this, &UPop_MainMenu::OnMenuButtonClicked);
    
    // 查找TierList容器（需要在蓝图中设置）
    UVerticalBox* TierList = Cast<UVerticalBox>(GetWidgetFromName(TEXT("TierList")));
    if (TierList)
    {
        // 添加到TierList
        UVerticalBoxSlot* Slot = TierList->AddChildToVerticalBox(NewButton);
        
        // 设置按钮样式
        Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        Slot->SetHorizontalAlignment(HAlign_Fill);
        Slot->SetVerticalAlignment(VAlign_Center);
        
        // 设置按钮边距
        Slot->SetPadding(FMargin(10.0f, 5.0f, 10.0f, 5.0f));
    }
    
    // 存储按钮信息
    m_arrButtonInfos.Add(ButtonInfo);
    m_mapButtonWidgets.Add(ButtonInfo.ButtonTag, NewButton);
    
    return NewButton;
}

void UPop_MainMenu::ClearAllButtons()
{
    // 查找TierList容器
    UVerticalBox* TierList = Cast<UVerticalBox>(GetWidgetFromName(TEXT("TierList")));
    if (TierList)
    {
        TierList->ClearChildren();
    }
    
    // 清空存储的数据
    m_arrButtonInfos.Empty();
    m_mapButtonWidgets.Empty();
}

bool UPop_MainMenu::RemoveButtonByTag(FName ButtonTag)
{
    if (!m_mapButtonWidgets.Contains(ButtonTag))
    {
        return false;
    }
    
    UButton* ButtonToRemove = m_mapButtonWidgets[ButtonTag];
    
    // 从TierList移除
    UVerticalBox* TierList = Cast<UVerticalBox>(GetWidgetFromName(TEXT("TierList")));
    if (TierList && ButtonToRemove)
    {
        TierList->RemoveChild(ButtonToRemove);
    }
    
    // 从数组中移除信息
    m_arrButtonInfos.RemoveAll([ButtonTag](const FMenuButtonInfo& Info) {
        return Info.ButtonTag == ButtonTag;
    });
    
    // 从映射中移除
    m_arrButtonInfos.Remove(ButtonTag);
    
    return true;
}

void UPop_MainMenu::OnMenuButtonClicked(FName ButtonTag)
{
    // 查找对应的按钮信息
    for (const auto& Info : ButtonInfos)
    {
        if (Info.ButtonTag == ButtonTag)
        {
            // 如果有回调事件，执行它
            if (Info.OnClicked.IsBound())
            {
                Info.OnClicked.Execute();
            }
            
            // 如果有关联的界面，切换到该界面
            if (Info.LinkedWidget)
            {
                SwitchToLinkedWidget(Info.LinkedWidget);
            }
            
            break;
        }
    }
}

void UPop_MainMenu::SwitchToLinkedWidget(UWidget* Widget)
{
    // 查找WidgetSwitcher
    UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(GetWidgetFromName(TEXT("MenuSwitcher")));
    if (WidgetSwitcher && Widget)
    {
        // 切换到对应的界面
        int32 WidgetIndex = WidgetSwitcher->GetChildIndex(Widget);
        if (WidgetIndex != INDEX_NONE)
        {
            WidgetSwitcher->SetActiveWidgetIndex(WidgetIndex);
        }
    }
}