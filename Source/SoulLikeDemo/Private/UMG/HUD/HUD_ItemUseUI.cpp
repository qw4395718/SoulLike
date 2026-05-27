#include "HUD_ItemUseUI.h"
#include "HUD_ItemUseSlot.h"
#include "Components/Image.h"
#include "Manager/SL_GameSaveSubsystem.h"
#include "Manager/DataTableManager.h"
#include "Manager/GlobalDelegatesManager.h"
#include "Table/ClassConfigInfoTable.h"
#include "UIManagerSubsystem.h"

UHUD_ItemUseUI::UHUD_ItemUseUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_currentIndex(0)
	, m_bDelegatesBound(false)
{
}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UHUD_ItemUseUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UHUD_ItemUseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 尝试从存档读取玩家职业ID
	int32 PlayerClassID = 1001; // 默认
	if (USL_GameSaveSubsystem* SaveSystem = USL_GameSaveSubsystem::Get(this))
	{
		if (USL_GameSaveData* SaveData = SaveSystem->GetSaveData())
		{
			PlayerClassID = SaveData->SavedPlayerClassID;
		}
	}

	// 初始化道具列表
	InitializeItems(PlayerClassID);

	// 绑定委托
	BindDelegates();
}

void UHUD_ItemUseUI::NativeDestruct()
{
	// 解除委托绑定
	UnbindDelegates();

	// 通知 UIManager 关闭（更新 ActiveWidgets 状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_None);
	}

	Super::NativeDestruct();
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_ItemUseUI::InitializeItems(int32 InPlayerClassID)
{
	m_itemList.Empty();
	m_itemCountMap.Empty();
	m_currentIndex = 0;

	// 从 DataTableManager 获取职业配置表
	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		RefreshSlots();
		return;
	}

	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(
		DTManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));

	if (!ClassTable)
	{
		RefreshSlots();
		return;
	}

	// 读取职业配置中的快捷栏道具列表
	FClassConfigInfo ClassConfig;
	if (!ClassTable->GetClassConfig(InPlayerClassID, ClassConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_ItemUseUI::InitializeItems - Class config not found for ID: %d"), InPlayerClassID);
		RefreshSlots();
		return;
	}

	// 拷贝道具列表 & 初始化数量缓存
	m_itemList = ClassConfig.SlotItems;
	for (const FSlotItemInfo& SlotItem : m_itemList)
	{
		if (SlotItem.ItemID != NAME_None && SlotItem.InitialCount > 0)
		{
			m_itemCountMap.Add(SlotItem.ItemID, SlotItem.InitialCount);
		}
	}

	// 刷新界面
	RefreshSlots();

	UE_LOG(LogTemp, Log, TEXT("UHUD_ItemUseUI::InitializeItems - Loaded %d items for class %d"),
		m_itemList.Num(), InPlayerClassID);
}

void UHUD_ItemUseUI::SelectPrevious()
{
	int32 ItemCount = m_itemList.Num();
	if (ItemCount <= 1)
	{
		return;
	}

	// 循环切换到上一个
	m_currentIndex = (m_currentIndex - 1 + ItemCount) % ItemCount;
	RefreshSlots();
}

void UHUD_ItemUseUI::SelectNext()
{
	int32 ItemCount = m_itemList.Num();
	if (ItemCount <= 1)
	{
		return;
	}

	// 循环切换到下一个
	m_currentIndex = (m_currentIndex + 1) % ItemCount;
	RefreshSlots();
}

FName UHUD_ItemUseUI::GetCurrentItemID() const
{
	if (!m_itemList.IsValidIndex(m_currentIndex))
	{
		return NAME_None;
	}

	FName ItemID = m_itemList[m_currentIndex].ItemID;

	// 检查数量是否大于 0
	const int32* Count = m_itemCountMap.Find(ItemID);
	if (Count && *Count > 0)
	{
		return ItemID;
	}

	return NAME_None;
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UHUD_ItemUseUI::RefreshSlots()
{
	int32 TotalItems = m_itemList.Num();

	auto GetItemCount = [this](const FName& InItemID) -> int32
	{
		const int32* Count = m_itemCountMap.Find(InItemID);
		return Count ? *Count : 0;
	};

	auto SetSlotData = [this, &GetItemCount](UHUD_ItemUseSlot* InSlot, int32 InIndex)
	{
		if (!InSlot)
		{
			return;
		}

		if (m_itemList.IsValidIndex(InIndex))
		{
			FName ItemID = m_itemList[InIndex].ItemID;
			int32 Count = GetItemCount(ItemID);

			if (Count > 0)
			{
				InSlot->SetItemInfo(ItemID, Count);
			}
			else
			{
				InSlot->ClearSlot();
			}
		}
		else
		{
			InSlot->ClearSlot();
		}
	};

	if (TotalItems == 0)
	{
		// 无可使用道具，全部清空
		if (m_prevSlot)    m_prevSlot->ClearSlot();
		if (m_currentSlot) m_currentSlot->ClearSlot();
		if (m_nextSlot)    m_nextSlot->ClearSlot();
		return;
	}

	// 计算三个槽位对应的索引
	int32 PrevIndex = (m_currentIndex - 1 + TotalItems) % TotalItems;
	int32 NextIndex = (m_currentIndex + 1) % TotalItems;

	// 设置数据
	SetSlotData(m_prevSlot, PrevIndex);
	SetSlotData(m_currentSlot, m_currentIndex);
	SetSlotData(m_nextSlot, NextIndex);

	// 设置选中状态：只有当前槽高亮
	if (m_prevSlot)    m_prevSlot->SetSelected(false);
	if (m_currentSlot) m_currentSlot->SetSelected(true);
	if (m_nextSlot)    m_nextSlot->SetSelected(false);
}

void UHUD_ItemUseUI::BindDelegates()
{
	if (m_bDelegatesBound)
	{
		return;
	}

	UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this);
	if (!DelegateMgr)
	{
		return;
	}

	DelegateMgr->OnItemCountChanged.AddUObject(this, &UHUD_ItemUseUI::OnItemCountChangedHandle);
	DelegateMgr->OnItemUsed.AddUObject(this, &UHUD_ItemUseUI::OnItemUsedHandle);

	m_bDelegatesBound = true;

	UE_LOG(LogTemp, Log, TEXT("UHUD_ItemUseUI::BindDelegates - Delegates bound"));
}

void UHUD_ItemUseUI::UnbindDelegates()
{
	if (!m_bDelegatesBound)
	{
		return;
	}

	UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this);
	if (DelegateMgr)
	{
		DelegateMgr->OnItemCountChanged.RemoveAll(this);
		DelegateMgr->OnItemUsed.RemoveAll(this);
	}

	m_bDelegatesBound = false;

	UE_LOG(LogTemp, Log, TEXT("UHUD_ItemUseUI::UnbindDelegates - Delegates unbound"));
}

void UHUD_ItemUseUI::OnItemCountChangedHandle(AActor* InOwnerActor, FName InItemID, int32 InNewCount)
{
	// 更新本地数量缓存
	if (InNewCount > 0)
	{
		m_itemCountMap.Add(InItemID, InNewCount);
	}
	else
	{
		m_itemCountMap.Remove(InItemID);
	}

	// 如果当前选中的道具已用完且还有更多可用的，自动跳到下一个
	if (InItemID == GetCurrentItemID() && InNewCount <= 0)
	{
		int32 TotalItems = m_itemList.Num();
		for (int32 Offset = 1; Offset < TotalItems; ++Offset)
		{
			int32 CheckIndex = (m_currentIndex + Offset) % TotalItems;
			FName CheckID = m_itemList[CheckIndex].ItemID;
			const int32* Count = m_itemCountMap.Find(CheckID);
			if (Count && *Count > 0)
			{
				m_currentIndex = CheckIndex;
				break;
			}
		}
	}

	// 刷新界面
	RefreshSlots();
}

void UHUD_ItemUseUI::OnItemUsedHandle(AActor* InUserActor, FName InItemID)
{
	// 道具被使用后，数量 -1
	const int32* CurrentCount = m_itemCountMap.Find(InItemID);
	if (CurrentCount)
	{
		int32 NewCount = *CurrentCount - 1;
		OnItemCountChangedHandle(InUserActor, InItemID, NewCount);
	}
}
