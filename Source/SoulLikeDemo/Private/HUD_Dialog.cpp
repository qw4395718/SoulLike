// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_Dialog.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/FontMeasure.h"


UHUD_Dialog::UHUD_Dialog(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_Dialog::FakeInit()
{
	FString tmpName = TEXT("原神");
	FString tmpContent = TEXT("《原神》是由中国游戏公司米哈游（miHoYo）自主研发的一款开放世界动作角色扮演游戏，于2020年9月正式全球上线，支持iOS、Android、PC、PlayStation等多平台，并计划登陆Nintendo Switch。游戏以精美的画面、丰富的探索内容和深度的角色养成系统闻名，在全球范围内拥有极高的人气。一、游戏核心特色/n开放世界探索/n游戏地图庞大且多样化，包含草原、雪山、沙漠、海洋、奇幻国度等区域，鼓励玩家自由探索。/n世界中隐藏着大量谜题、宝箱、秘境和随机事件，探索奖励丰富。");
	SetDialogText(tmpName, tmpContent);
}

void UHUD_Dialog::SetDialogText(const FString& labelText, const FString& descText)
{
	m_pawnName = labelText;
	m_dialogContent = descText;
	m_currentPageIndex = 0;
	m_arrDialogSplit.Empty();
	m_characterWidthCache.Empty();

	// 分割文本
	SplitTextIntoSections();

	// 更新多页状态
	m_bIsMultiPage = (m_arrDialogSplit.Num() > 1);

	// 显示第一页
	if (m_arrDialogSplit.Num() > 0)
	{
		ShowPage(0);
	}
}

void UHUD_Dialog::ShowDialog()
{
	SetVisibility(ESlateVisibility::Visible);
	AddToViewport();
}

void UHUD_Dialog::CloseDialog()
{
	RemoveFromParent();
}

void UHUD_Dialog::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮点击事件
	if (m_clickButton)
	{
		m_clickButton->OnClicked.AddDynamic(this, &UHUD_Dialog::OnClickButtonClicked);
	}

	// 初始显示第一页
	if (m_arrDialogSplit.Num() > 0)
	{
		ShowPage(0);
	}
}

void UHUD_Dialog::NativeDestruct()
{
	// 移除委托绑定
	if (m_clickButton)
	{
		m_clickButton->OnClicked.RemoveDynamic(this, &UHUD_Dialog::OnClickButtonClicked);
	}

	Super::NativeDestruct();
}

void UHUD_Dialog::SplitTextIntoSections()
{
	if (m_dialogContent.IsEmpty())
	{
		m_arrDialogSplit.Add(FString(""));
		return;
	}

	if (m_bUseSmartSplitting && m_dialogText)
	{
		// 使用智能分页（基于宽度）
		SmartSplitTextByWidth();
	}
	else
	{
		// 使用简化分页（基于行数）
		SplitTextByEstimatedLines();
	}
}

void UHUD_Dialog::ShowPage(int32 PageIndex)
{
	if (!m_dialogText || !m_pawnNameText || !m_arrDialogSplit.IsValidIndex(PageIndex))
	{
		return;
	}

	m_currentPageIndex = PageIndex;

	FText DisplayTextTitle = FText::FromString(m_pawnName);
	m_pawnNameText->SetText(DisplayTextTitle);

	// 设置文本内容
	FText DisplayText = FText::FromString(m_arrDialogSplit[PageIndex]);
	m_dialogText->SetText(DisplayText);
}

bool UHUD_Dialog::IsChineseChar(TCHAR Char) const
{
	uint32 CodePoint = static_cast<uint32>(Char);

	// 常用汉字范围
	return (CodePoint >= 0x4E00 && CodePoint <= 0x9FFF) ||     // 基本汉字
		(CodePoint >= 0x3400 && CodePoint <= 0x4DBF) ||     // 扩展A
		(CodePoint >= 0x20000 && CodePoint <= 0x2A6DF) ||   // 扩展B
		(CodePoint >= 0x2A700 && CodePoint <= 0x2B73F) ||   // 扩展C
		(CodePoint >= 0x2B740 && CodePoint <= 0x2B81F) ||   // 扩展D
		(CodePoint >= 0x2B820 && CodePoint <= 0x2CEAF) ||   // 扩展E
		(CodePoint >= 0x2CEB0 && CodePoint <= 0x2EBEF) ||   // 扩展F
		(CodePoint >= 0x30000 && CodePoint <= 0x3134F);     // 扩展G
}

bool UHUD_Dialog::IsGoodBreakChar(TCHAR Char) const
{
	// 空格、标点符号等
	return Char == TEXT(' ') ||
		Char == TEXT(',') ||
		Char == TEXT('.') ||
		Char == TEXT('!') ||
		Char == TEXT('?') ||
		Char == TEXT('，') ||
		Char == TEXT('。') ||
		Char == TEXT('！') ||
		Char == TEXT('？') ||
		Char == TEXT('、') ||
		Char == TEXT('；') ||
		Char == TEXT('：');
}

float UHUD_Dialog::GetCharWidth(TCHAR Char, const FSlateFontInfo& FontInfo) const
{
	// 使用缓存提高性能
	float* CachedWidth = m_characterWidthCache.Find(Char);
	if (CachedWidth)
	{
		return *CachedWidth;
	}

	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	FString SingleChar(1, &Char);
	float Width = FontMeasure->Measure(SingleChar, FontInfo).X;

	// 缓存结果
	m_characterWidthCache.Add(Char, Width);

	return Width;
}

int32 UHUD_Dialog::EstimateLinesForText(const FString& Text) const
{
	if (Text.IsEmpty())
	{
		return 0;
	}

	int32 ChineseCount = 0;
	int32 EnglishCount = 0;

	for (TCHAR Char : Text)
	{
		if (IsChineseChar(Char))
		{
			ChineseCount++;
		}
		else if (FChar::IsAlnum(Char) || Char == TCHAR(' '))
		{
			EnglishCount++;
		}
		else if (Char == TCHAR('\t'))
		{
			// Tab算作4个英文字符
			EnglishCount += 4;
		}
	}

	// 估算：每行大约20个中文字符或40个英文字符
	// 这是一个粗略估算，实际需要根据字体大小调整
	float EstimatedLines = (ChineseCount / 20.0f) + (EnglishCount / 40.0f);

	// 至少为1行
	return FMath::Max(1, FMath::CeilToInt(EstimatedLines));
}

void UHUD_Dialog::OnClickButtonClicked()
{
	if (!m_bIsMultiPage)
	{
		// 单页直接关闭
		CloseDialog();
		return;
	}

	// 检查是否为最后一页
	if (m_currentPageIndex >= m_arrDialogSplit.Num() - 1)
	{
		CloseDialog();
	}
	else
	{
		// 显示下一页
		ShowPage(m_currentPageIndex + 1);
	}
}

void UHUD_Dialog::SmartSplitTextByWidth()
{
	if (!m_dialogText) return;

	TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	
	FSlateFontInfo FontInfo = m_dialogText->Font;

	int32 CurrentIndex = 0;
	int32 TextLength = m_dialogContent.Len();

	while (CurrentIndex < TextLength)
	{
		FString CurrentPage;
		float CurrentWidth = 0.0f;
		int32 LastGoodBreak = CurrentIndex;
		int32 LastSpaceIndex = -1;
		bool bLineBreakFound = false;

		for (int32 i = CurrentIndex; i < TextLength; i++)
		{
			TCHAR CurrentChar = m_dialogContent[i];

			// 检查换行符
			if (CurrentChar == TCHAR('\n'))
			{
				CurrentPage = m_dialogContent.Mid(CurrentIndex, i - CurrentIndex);
				CurrentIndex = i + 1;
				bLineBreakFound = true;
				break;
			}

			FString SingleChar = FString(1, &CurrentChar);
			float CharWidth = GetCharWidth(CurrentChar, FontInfo);

			// 检查是否超过宽度限制
			if (CurrentWidth + CharWidth > m_maxPageWidth)
			{
				// 找到合适的断点
				if (LastSpaceIndex != -1 && LastSpaceIndex > CurrentIndex)
				{
					// 在空格处断开
					CurrentPage = m_dialogContent.Mid(CurrentIndex, LastSpaceIndex - CurrentIndex);
					CurrentIndex = LastSpaceIndex + 1;
				}
				else if (LastGoodBreak > CurrentIndex)
				{
					// 在上一个合适的字符处断开
					CurrentPage = m_dialogContent.Mid(CurrentIndex, LastGoodBreak - CurrentIndex);
					CurrentIndex = LastGoodBreak;
				}
				else
				{
					// 强制在当前字符前断开
					CurrentPage = m_dialogContent.Mid(CurrentIndex, i - CurrentIndex);
					CurrentIndex = i;
				}
				break;
			}

			// 记录可能的断点位置
			if (IsGoodBreakChar(CurrentChar))
			{
				LastSpaceIndex = i;
			}

			// 记录中文字符边界
			if (IsChineseChar(CurrentChar))
			{
				LastGoodBreak = i + 1;
			}

			CurrentWidth += CharWidth;

			// 处理最后一个字符
			if (i == TextLength - 1)
			{
				CurrentPage = m_dialogContent.Mid(CurrentIndex, TextLength - CurrentIndex);
				CurrentIndex = TextLength;
				break;
			}
		}

		// 处理换行符的情况
		if (bLineBreakFound)
		{
			if (!CurrentPage.IsEmpty())
			{
				m_arrDialogSplit.Add(CurrentPage);
			}
			continue;
		}

		// 移除开头的空格
		CurrentPage.TrimStartInline();

		// 添加到结果
		if (!CurrentPage.IsEmpty())
		{
			m_arrDialogSplit.Add(CurrentPage);
		}
	}

	// 如果没有分页，添加完整文本
	if (m_arrDialogSplit.Num() == 0)
	{
		m_arrDialogSplit.Add(m_dialogContent);
	}
}

void UHUD_Dialog::SplitTextByEstimatedLines()
{
	TArray<FString> Lines;
	m_dialogContent.ParseIntoArrayLines(Lines, false);

	FString CurrentPage;
	int32 CurrentLineCount = 0;

	for (const FString& Line : Lines)
	{
		int32 EstimatedLines = EstimateLinesForText(Line);

		if (CurrentLineCount + EstimatedLines > m_maxLinesPerPage && !CurrentPage.IsEmpty())
		{
			// 开始新的一页
			m_arrDialogSplit.Add(CurrentPage);
			CurrentPage = Line;
			CurrentLineCount = EstimatedLines;
		}
		else
		{
			// 添加到当前页
			if (!CurrentPage.IsEmpty())
			{
				CurrentPage += TEXT("\n");
			}
			CurrentPage += Line;
			CurrentLineCount += EstimatedLines;
		}
	}

	// 添加最后一页
	if (!CurrentPage.IsEmpty())
	{
		m_arrDialogSplit.Add(CurrentPage);
	}

	// 如果没有分页，添加完整文本
	if (m_arrDialogSplit.Num() == 0)
	{
		m_arrDialogSplit.Add(m_dialogContent);
	}
}

