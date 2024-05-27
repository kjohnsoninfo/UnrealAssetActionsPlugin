// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab) {}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
#pragma region TitleSlot
	TSharedRef<STextBlock> ConstructTitleTextForTab(const FString& TextContent);
	TSharedRef<SButton> ConstructHelpButton();
	FReply OnHelpButtonClicked();
#pragma endregion


	FSlateFontInfo GetEmbossedFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

};

