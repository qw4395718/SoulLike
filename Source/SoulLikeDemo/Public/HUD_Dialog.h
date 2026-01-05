// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_Dialog.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_Dialog : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_Dialog(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 单元测试相关
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void FakeInit();

	/** 设置要显示的文本 */
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void SetDialogText(const FString& pawnNameText,const FString& dialogText);

	/** 显示对话框 */
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ShowDialog();

	/** 关闭对话框 */
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void CloseDialog();



protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 继承
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


	/** 智能分页文本 */
	void SplitTextIntoSections();

	/** 显示指定页码 */
	void ShowPage(int32 pageIndex);

	/** 检查是否为中文字符 */
	bool IsChineseChar(TCHAR charText) const;

	/** 检查是否为良好的断点字符 */
	bool IsGoodBreakChar(TCHAR charText) const;

	/** 获取字符宽度 */
	float GetCharWidth(TCHAR charText, const FSlateFontInfo& fontInfo) const;

	/** 估算文本行数 */
	int32 EstimateLinesForText(const FString& text) const;
	
	// 鼠标点击行为响应
	UFUNCTION()
	void OnClickButtonClicked();

	// 智能分页（基于宽度）
	void SmartSplitTextByWidth();

	// 简化分页（基于行数）
	void SplitTextByEstimatedLines();

protected:
	/************************************************************************/
	/* 内部变量-控件                                                                   */
	/************************************************************************/
	// 角色姓名控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_pawnNameText;

	// 对话内容控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_dialogText;

	// 不可见按钮控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UButton* m_clickButton;

private:
	/************************************************************************/
	/* 内部变量-数据                                                                     */
	/************************************************************************/
	// 角色姓名
	FString m_pawnName;
	// 文本总内容
	FString m_dialogContent;
	// 当前页码
	int m_currentPageIndex;
	// 总页码
	int m_maxPageNum;
	// 单页宽度
	int m_maxPagePageWidth;
	// 分页后的数据
	TArray<FString> m_arrDialogSplit;
	// 是否是多页显示
	bool m_bIsMultiPage;
	// 字体测量缓存
	mutable TMap<TCHAR, float> m_characterWidthCache;
	// 委托绑定 
	FDelegateHandle OnClickedHandle;

	/************************************************************************/
	/* 内部变量-配置                                                                     */
	/************************************************************************/
	/** 属性配置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (AllowPrivateAccess = "true"))
	float m_maxPageWidth = 1300.0f * 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (AllowPrivateAccess = "true"))
	int32 m_maxLinesPerPage = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (AllowPrivateAccess = "true"))
	bool m_bUseSmartSplitting = true;
};

