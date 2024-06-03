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
	
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView; // Ptr since it can be null before construction

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	void RefreshAssetListView();

#pragma endregion

#pragma region RowsInListView
	TArray<TSharedPtr<FAssetData>> AssetsDataFromManager;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> CheckedAssetsToDelete;

	TSharedRef<ITableRow> OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	
	TSharedRef<SCheckBox> ConstructCheckBoxes(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState CheckBoxState, TSharedPtr<FAssetData> ClickedAssetData);
	
	TSharedRef<STextBlock> ConstructTextForRow(const FString& RowText);

	TSharedRef<SButton> ConstructDeleteButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
#pragma endregion

#pragma region ButtonSlot
	TSharedRef<SButton> ConstructButtonForSlot(const FString& ButtonName);
	TSharedRef<STextBlock> ConstructTextForButtonSlot(const FString& ButtonName);

	void AssignButtonClickFns(const FString& ButtonName);
	FReply OnDeleteSelectedButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();
#pragma endregion

	FSlateFontInfo SharedTextFont;

	FSlateFontInfo GetEmbossedFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

};

