// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab) {}

		/**
		 * Argument used to pass asset data between manager and widget
		 * 
		 * @note AssetsDataFromManager: Array of asset data from the manager
		 * This argument name matches the FArg in the OnSpawn fn in AssetActionsManager.
		 * 
		 * The type must match what the ListItemSource fn in ConstructAssetListView expects.
		 */
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataFromManager) 

	SLATE_END_ARGS()

public:
	/**
	 * Construct Advanced Deletion Widget
	 * 
	 * @param InArgs: defined SLATE_ARGUMENTS
	 * Allows access to assign data from slate arguments to class variables
	 */
	void Construct(const FArguments& InArgs);

private:
#pragma region TitleSlot
	
	/** Display a title at the top of the widget */
	TSharedRef<STextBlock> ConstructTitleTextForTab(const FString& TitleText);

	/** Construct Help Button and when clicked, go to documentation webpage */
	TSharedRef<SButton> ConstructHelpButton();
	FReply OnHelpButtonClicked();

#pragma endregion

#pragma region ListView
	
	/**
	 * Pointer to the SListView created when widget is constructed
	 * 
	 * @note: This is be a ptr since it can be null before construction.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;

	/** Construct SListView to display all assets in selected folder */
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();

	/** Refresh SListView when an asset is deleted to ensure list is always up to date */
	void RefreshAssetListView();

#pragma endregion

#pragma region RowsInListView

	/** Array to hold all asset data passed in by the AssetActionsManager
	* @note: This is assigned to the InArgs of the Construct fn.
	*/
	TArray<TSharedPtr<FAssetData>> AssetsDataFromManager;

	/** Array to hold all constructed CheckBoxes when widget is constructed
	* @note: The size of this array is dependent on the number of assets in the selected folder
	* and subfolders. Array is important for select and deselect fns.
	*/
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;

	/** Array to hold all assets that are checked in the asset list view.
	* @note: Array is important for delete selected fn.
	*/
	TArray<TSharedPtr<FAssetData>> CheckedAssetsToDelete;

	/** Generate a row in the list view for every asset found in the selected folders
	* @note: Array is important for delete selected fn.
	*/
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

