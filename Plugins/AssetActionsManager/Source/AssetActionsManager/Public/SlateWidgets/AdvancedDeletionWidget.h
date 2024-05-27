// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab) {}
		
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataFromManager) // type matches what row item source expects

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
#pragma region TitleSlot

	TSharedRef<STextBlock> ConstructTitleTextForTab(const FString& TitleText);
	TSharedRef<SButton> ConstructHelpButton();
	FReply OnHelpButtonClicked();

#pragma endregion

#pragma region ListView

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();

#pragma endregion

#pragma region RowsInListView
	TArray<TSharedPtr<FAssetData>> AssetDataArrayFromManager;
	TArray<TSharedPtr<FAssetData>> AssetDataArrayToDisplay;

	TSharedRef<ITableRow> OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<STextBlock> ConstructTextForRow(const FString& RowText);
#pragma endregion



	FSlateFontInfo GetEmbossedFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

};

