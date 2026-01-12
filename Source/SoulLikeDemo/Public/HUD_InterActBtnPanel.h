// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_InterActBtnPanel.generated.h"

class UUI_IconSlot;
class UUI_InterActButton;
class UTexture2D;
class UVerticalBox;

// 交互组件相关数据结构
struct FInterActOptionInfo
{
	int32				Index;
	UTexture2D*		OptionIcon;
	FString			OptionText;
};

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_InterActBtnPanel : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 所有数据更新
	void UpdateBatch(const TArray<FInterActOptionInfo>& options);

	// 单项数据更新
	void UpdateTarget(const FInterActOptionInfo& options);

	UFUNCTION(BlueprintCallable)
		void ClearAllOptions();

	UFUNCTION(BlueprintCallable)
		void SetTargetOptionSelected(int32 Index);

	// 按钮点击事件
	UFUNCTION(BlueprintCallable)
		void OnButtonClicked(int32 Index);

	UFUNCTION(BlueprintCallable)
		void SetVisible(bool bVisible);
	
	UFUNCTION(BlueprintCallable)
	void UpdateVisibleSlots(int32 oldFirstIndex, int32 oldLastIndex);

	// 初始化虚拟化
	UFUNCTION(BlueprintCallable)
	void InitializeVirtualization(int32 TotalItemCount);

	// 从池中获取或创建槽位
	UUI_InterActButton* GetOrCreateSlot();

	// 回收槽位到池中
	void ReturnSlotToPool(UUI_InterActButton* Slot);


protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 单元测试相关
	void FakeInit();

	// 重写原生鼠标滚轮事件
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 鼠标滚动行为处理
	UFUNCTION(BlueprintCallable, Category="Scroll")
	void HandleScroll(float wheelDelta);

	UFUNCTION(BlueprintCallable)
		void UpdateVisibleRange();

	void UpdateSlotPositions();

	// 将指定数据源索引填入新获取的控件中并纳入可视区域缓存
	void AddSlotForIndex(int32 Index);

	// 将指定数据源索引填入新获取的控件中并纳入可视区域缓存,插到头部
	void AddSlotAtIndexZero(int32 Index);

	// 获取数据源指定索引的数据
	const FInterActOptionInfo* GetItemAtIndex(int32 Index);

	// 获取基础控件配置高度
	float GetSlotHeight();

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "interActPanel")
		TSubclassOf<UUI_InterActButton> m_interActBtnClass;

	// 控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UVerticalBox* m_interActBtnsContainer;

	// 交互控件管理器-虚拟化(可视区域缓存)
	UPROPERTY()
		TArray<UUI_InterActButton*> m_visibleSlots;

	// 交互控件管理器-虚拟化(池)
	UPROPERTY()
		TArray<UUI_InterActButton*> m_slotPool;


	// 本地数据
		TArray<FInterActOptionInfo> m_interActDataArr;

		// 当前显示范围
		int32 FirstVisibleIndex = 0;
		int32 LastVisibleIndex = 0;

	// 当前选中索引
	UPROPERTY(BlueprintReadOnly, Category = "State")
		int32 SelectedIndex = -1;

	private:
	/************************************************************************/
	/* 内部变量-配置                                                                     */
	/************************************************************************/
	// 滚动速度控制
	UPROPERTY(EditAnywhere, Category = "Scroll")
		float scrollSensitivity = 40.0f;

	// 当前滚动偏移
	float currentScrollOffset = 0.0f;

};
