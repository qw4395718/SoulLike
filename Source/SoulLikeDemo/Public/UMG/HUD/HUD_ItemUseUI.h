#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ItemUseUI.generated.h"

class UImage;
class UHUD_ItemUseSlot;

/**
 * 道具使用界面Widget
 *
 * 应用场景：仅显示在战斗关卡中（由外部系统控制何时创建/打开）
 * 非战斗关卡（如大厅）不显示
 *
 * 布局（右下角，从左到右）：
 *   [切换上一个按键图标] [上一个道具槽] [当前选中道具槽] [下一个道具槽] [切换下一个按键图标]
 *
 * 数据来源：
 *   进入关卡时根据玩家职业从 FClassConfigInfo::SlotItems 读取
 *   运行时通过 GlobalDelegatesManager::OnItemCountChanged / OnItemUsed 更新
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_ItemUseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UHUD_ItemUseUI(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 根据玩家职业ID初始化道具列表（可由外部传入，也可在内部读存档自动获取）
	UFUNCTION(BlueprintCallable, Category = "ItemUseUI")
	void InitializeItems(int32 InPlayerClassID);

	// 切换到上一个道具
	UFUNCTION(BlueprintCallable, Category = "ItemUseUI")
	void SelectPrevious();

	// 切换到下一个道具
	UFUNCTION(BlueprintCallable, Category = "ItemUseUI")
	void SelectNext();

	// 获取当前选中的道具ID（NAME_None 表示无可使用道具）
	UFUNCTION(BlueprintPure, Category = "ItemUseUI")
	FName GetCurrentItemID() const;

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 根据当前索引刷新三个槽位的显示
	void RefreshSlots();

	// 绑定 GlobalDelegatesManager 的委托回调
	void BindDelegates();

	// 解除所有委托绑定
	void UnbindDelegates();

	// 道具数量变更回调
	void OnItemCountChangedHandle(AActor* InOwnerActor, FName InItemID, int32 InNewCount);

	// 道具使用回调
	void OnItemUsedHandle(AActor* InUserActor, FName InItemID);

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 切换上一个道具的按键图标（如键盘 "Q"）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseUI")
	UImage* m_prevKeyImage;

	// 切换下一个道具的按键图标（如键盘 "E"）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseUI")
	UImage* m_nextKeyImage;

	// 上一个道具槽
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseUI")
	UHUD_ItemUseSlot* m_prevSlot;

	// 当前选中的道具槽
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseUI")
	UHUD_ItemUseSlot* m_currentSlot;

	// 下一个道具槽
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseUI")
	UHUD_ItemUseSlot* m_nextSlot;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/

	// 快捷栏道具列表（从 FClassConfigInfo 读取）
	UPROPERTY()
	TArray<FSlotItemInfo> m_itemList;

	// 运行时实际数量缓存（道具ID -> 当前数量，动态变化）
	UPROPERTY()
	TMap<FName, int32> m_itemCountMap;

	// 当前选中的道具索引
	int32 m_currentIndex;

	// 是否已绑定委托
	bool m_bDelegatesBound;
};
