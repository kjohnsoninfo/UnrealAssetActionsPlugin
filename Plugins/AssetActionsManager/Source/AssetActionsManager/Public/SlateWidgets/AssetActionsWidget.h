// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"

namespace AssetActionsColumns
{
	static const FName Checkbox(TEXT("Checkbox")); // unsortable
	static const FName Class(TEXT("Class"));
	static const FName Name(TEXT("Name")); 
	static const FName Path(TEXT("Path")); 
	static const FName RefCount(TEXT("RefCount")); 
	static const FName Rename(TEXT("Rename")); // unsortable
}

class SAssetActionsTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAssetActionsTab) {}

		/**
		 * Argument used to pass asset data between manager and widget
		 * 
		 * @note AssetsDataFromManager: Array of asset data from the manager
		 * This argument name matches the FArg in the OnSpawn fn in AssetActionsManager.
		 * 
		 * The type must match what the ListItemSource fn in ConstructAssetListView expects.
		 */
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AllAssetsDataFromManager) 

		/** Argument for the folder path that user selected when spawning tab */
		SLATE_ARGUMENT(TArray<FString>, SelectedFoldersPaths)

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
	
	/** Display a label at the top of the widget */
	TSharedRef<STextBlock> ConstructLabelText(const FString& LabelText);

	/** Display title at the top center of the widget */
	TSharedRef<STextBlock> ConstructTitleText(const FString& TitleText);

	/** Construct Help Button and when clicked, go to documentation webpage */
	TSharedRef<SButton> ConstructHelpButton();
	FReply OnHelpButtonClicked();

#pragma endregion

#pragma region FilterSlot

	/** Array that contains criteria to filter on */
	TArray<TSharedPtr<FString>> FilterListItems;
	
	/** 
	 * Text to display in the "selected item" area of the ComboBox 
	 * 
	 * @note: This defaults to List All Assets
	 */
	TSharedPtr<STextBlock> ComboBoxDisplayedText;

	/** Construct ComboBox that allows to user to filter asset list vew */
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructFilterComboBox();

	/** Construct widgets to contain the filter items in the ComboBox dropdown options*/
	TSharedRef<SWidget> OnGenerateFilterItem(TSharedPtr<FString> FilterItem);

	/** Delegate function to apply filter criteria based on user selection */
	void OnFilterSelectionChanged(TSharedPtr<FString> SelectedFilter, ESelectInfo::Type InSelectInfo);
	
	/** Construct Refresh Button that allows a user to manually refresh the asset list view */
	TSharedRef<SButton> ConstructRefreshButton();
	FReply OnRefreshButtonClicked();

#pragma endregion


#pragma region ListView
	
	/** 
	 * Pointer to the SListView created when widget is constructed
	 * 
	 * @note: This is be a ptr since it can be null before construction.
	 */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;

	/** Checkbox that appears in the header row and triggers select/deselect all fns */
	TSharedPtr<SCheckBox> HeaderCheckBox;

	/** Currently selected column to sort with; default = Name */
	FName SortByColumn;

	/** Currently selected sorting mode; default = Ascending */
	EColumnSortMode::Type SortMode;

	/** Construct SListView to display all assets in selected folder */
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();

	/** Navigate to selected asset in content browser when its row is doubleclicked */
	void OnRowDoubleClick(TSharedPtr<FAssetData> ClickedAssetData);

	/** Construct header row label for each column */
	TSharedRef<STextBlock> ConstructTextForHeaderRow(const FString& ColumnName);
	
	/** Construct a checkbox for header row that toggles between select all and deselect all */
	TSharedRef<SCheckBox> ConstructCheckBoxForHeaderRow();

	/** Delegate function that keeps track of checked and unchecked state of each row */
	void OnHeaderCheckBoxStateChanged(ECheckBoxState CheckBoxState);

	/** 
	 * Delegate function to get sort mode for each column
	 *
	 * @note: the second const declares a const fn which is useful for get fns
	 */
	EColumnSortMode::Type GetSortModeForColumn(const FName ColumnId) const;

	/** Delegate function to apply new sort criteria when changed by user */
	void OnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type InSortMode);

	/** Function that contains sorting logic for every sortable column */
	void UpdateSorting();
	
	/** Function sets the sort criteria when widget is first spawned */
	void DefaultSorting();

#pragma endregion

#pragma region RowsInListView

	/** 
	 * Array to hold all asset data passed in by the AssetActionsManager
	 * 
	 * @note: This is assigned to the InArgs of the Construct fn.
	 */
	TArray<TSharedPtr<FAssetData>> AllAssetsDataFromManager;

	/** 
	 * Array to hold all asset data displayed in the list view
	 * 
	 * @note: This is passed in as the list item source
	 */
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;

	/** Array to hold unused assets filtered by manager */
	TArray<TSharedPtr<FAssetData>> UnusedAssetsData;

	/** Array to hold unused assets filtered by manager */
	TArray<TSharedPtr<FAssetData>> DuplicatedNameAssetsData;

	/** 
	 * Array to hold all constructed CheckBoxes when widget is constructed
	 * 
	 * @note: The size of this array is dependent on the number of assets in the selected folder
	 * and subfolders. Array is important for select and deselect fns.
	 */
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;

	/** Array to hold all assets that are checked in the asset list view. */
	TArray<TSharedPtr<FAssetData>> CheckedAssets;

	/** Array to hold all assets that are not checked in the asset list view. */
	TArray<TSharedPtr<FAssetData>> UncheckedAssets;

	/** 
	 * Generate a row in the list view for every asset found in the selected folders
	 * 
	 * @param AssetDataToDisplay: AssetData specific to each row
	 * @note: AssetDataToDisplay is used to determine the clicked asset data when user 
	 *		  clicks on a specific row
	 */
	TSharedRef<ITableRow> OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, 
		const TSharedRef<STableViewBase>& OwnerTable);

	/** Construct a checkbox for each row in the list view */
	TSharedRef<SCheckBox> ConstructCheckBoxes(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/** Delegate function that keeps track of checked and unchecked state of each row */
	void OnCheckBoxStateChanged(ECheckBoxState CheckBoxState, TSharedPtr<FAssetData> ClickedAssetData);
	
	/** Construct text for each row to display asset properties such as class, name, and path */
	TSharedRef<STextBlock> ConstructTextForRow(const FString& RowText);

	/** Construct a rename button for each row in the list view */
	TSharedRef<SButton> ConstructRenameButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	/** Delegate function that opens a dialog box for the user to enter a new asset name */
	FReply OnRenameButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/** Call the rename fn from the manager and passes in the user chosen new name */
	void RenameAsset(const FString& NewName, const TSharedPtr<FAssetData>& AssetToRename);

#pragma endregion

#pragma region ButtonSlot
	/** Common function to construct buttons in the button slot */
	TSharedRef<SButton> ConstructButtonForSlot(const FString& ButtonName);

	/** Common function to construct text for the buttons in the button slot */
	TSharedRef<STextBlock> ConstructTextForButtonSlot(const FString& ButtonName);

	/** Assign different onClicked fns to each button based on button name */
	void AssignButtonClickFns(const FString& ButtonName);

	/** Delegate function that calls the manager delete fn to delete all checked assets */
	FReply OnDeleteSelectedButtonClicked();

	/** Delegate function that checks state of all checkboxes and toggles to checked */
	FReply OnDuplicateSelectedButtonClicked();

	/** Display a modal dialog to get user input for number of duplicates */
	int32 GetUserNumberForDuplicates();

	/** Delegate function that checks state of all checkboxes and toggles to unchecked */
	FReply OnDeselectAllButtonClicked();

#pragma endregion

#pragma region HelpfulInfoSlot

	/** 
	 * String containing message displayed for the asset count
	 * 
	 * @note: this is reassigned when widget is refreshed
	 */
	FString AssetCountMsg;

	/**
	 * Pointer to the SRichTextBlock created when widget is constructed
	 *
	 * @note: This allows for text to change dynamically when widget is refreshed
	 */
	TSharedPtr<SRichTextBlock> ConstructedAssetCountTextBlock;

	/** Array of folder paths that the user selected when spawning widget */
	TArray<FString> SelectedFoldersPaths;

	/** Construct rich text block to show the count of displayed assets in the list view */
	TSharedRef<SRichTextBlock> ConstructTextForAssetCount();

	/** Construct text block to show user selected folder paths */
	TSharedRef<STextBlock> ConstructTextForSelectedFolderPath();

#pragma endregion

#pragma region HelperFunctions

	/** Common font that defines shared properties in all textblocks for consistency */
	FSlateFontInfo SharedTextFont;

	/** Refresh widget to ensure text and list view are always up to date */
	void RefreshWidget();

	/** Helper function to update changes in the list view such as moving folders or adding assets  */
	void FilterAssetData();

	/** Helper function to remove asset data from arrays when deleted */
	void EnsureAssetDeletionFromLists(const TSharedPtr<FAssetData>& AssetDataToDelete);

	/** Helper function to get map of asset names and paths */
	TMultiMap<FString, FString> GetCheckBoxAssetMap(const TArray<TSharedPtr<FAssetData>> CheckBoxStateArray);

	/** Helper function to empty all data arrays associated with checkboxes */
	void ClearCheckedStates();

	/** Helper function to load asset action manager module */
	class FAssetActionsManagerModule& LoadManagerModule() const 
	{ return FModuleManager::LoadModuleChecked<FAssetActionsManagerModule>(TEXT("AssetActionsManager")); }

	/** Helper function to get UE Embossed font style */
	FSlateFontInfo GetEmbossedFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

	/** Helper function to get asset count message since it is dynamically reassigned based on displayed assets */
	FString GetAssetCountMsg() const { return FString::FromInt(DisplayedAssetsData.Num()) + TEXT(" assets"); }

#pragma endregion

};

