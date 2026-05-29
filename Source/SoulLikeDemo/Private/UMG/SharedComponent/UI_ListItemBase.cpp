#include "UI_ListItemBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "SL_Macros.h"

UUI_ListItemBase::UUI_ListItemBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_itemData(0)
	, m_bSelected(false)
{
	
}


void UUI_ListItemBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (m_mainButton != nullptr)
	{
		m_mainButton->OnClicked.AddDynamic(this,&UUI_ListItemBase::OnButtonClicked);
	}
	
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UUI_ListItemBase::SetDisplayText(const FText& InText)
{
	RETURN_IF_TRUE(m_titleText == nullptr);
	m_titleText->SetText(InText);
}

void UUI_ListItemBase::SetIcon(UTexture2D* InTexture)
{
	RETURN_IF_TRUE(m_iconImage == nullptr || InTexture == nullptr);
	m_iconImage->SetBrushFromTexture(InTexture, true);
}

void UUI_ListItemBase::SetItemData(int32 InData)
{
	m_itemData = InData;
}

int32 UUI_ListItemBase::GetItemData() const
{
	return m_itemData;
}

void UUI_ListItemBase::SetItemEnabled(bool bInEnabled)
{
	RETURN_IF_TRUE(m_mainButton == nullptr);

	m_mainButton->SetIsEnabled(bInEnabled);

	if (m_titleText)
	{
		m_titleText->SetColorAndOpacity(
			bInEnabled ? FLinearColor::White : FLinearColor(0.3f, 0.3f, 0.3f));
	}
}

void UUI_ListItemBase::SetSelected_Implementation(bool bInSelected)
{
	m_bSelected = bInSelected;
	// 选中态视觉由蓝图扩展
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UUI_ListItemBase::OnButtonClicked()
{
	if (OnItemClicked.IsBound())
	{
		OnItemClicked.Execute(m_itemData);
	}
}
