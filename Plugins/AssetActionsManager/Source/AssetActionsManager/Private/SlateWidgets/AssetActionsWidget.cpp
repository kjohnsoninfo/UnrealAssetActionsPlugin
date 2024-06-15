// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AssetActionsWidget.h"

#include "AssetActionsManager.h"
#include "DebugHelper.h"
#include "Dialog/SCustomDialog.h"
#include "SlateWidgets/RenameAssetDialog.h"
#include "SlateWidgets/ReplaceStringDialog.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "SAssetActionsTab"

#define AddPrefixSelected TEXT("Add Prefixes for Selected")
#define DeleteSelected TEXT("Delete Selected")
#define DuplicateSelected TEXT("Duplicate Selected")
#define ReplaceStringSelected TEXT("Replace String for Selected")

#define ListAll TEXT("List All Assets")
#define ListUnused TEXT("List Unused Assets")
#define ListDuplicate TEXT("List Duplicate Name Assets")
#define ListNoPrefix TEXT("List Assets with No Prefix")

void SAssetActionsTab::Construct(const FArguments& InArgs)
/*
	Widget constructor that initializes all variables and Slate components
*/
{
	bCanSupportFocus = true;

	// Ensure clean slate when constructed
	FilterListItems.Empty();

	// Grab data from manager
	AllAssetsDataFromManager = InArgs._AllAssetsDataFromManager;
	DisplayedAssetsData = AllAssetsDataFromManager; // set display data to all by default
	UncheckedAssets = DisplayedAssetsData; // all assets are unchecked on spawn
	SelectedFoldersPaths = InArgs._SelectedFoldersPaths; 

	AssetCountMsg = GetAssetCountMsg();

	SharedTextFont = GetEmbossedFont();
	SharedTextFont.Size = 12;

	// Add filters to combobox
	FilterListItems.Add(MakeShared<FString>(ListAll));
	FilterListItems.Add(MakeShared<FString>(ListUnused));
	FilterListItems.Add(MakeShared<FString>(ListDuplicate));
	FilterListItems.Add(MakeShared<FString>(ListNoPrefix));

	ChildSlot
		[
			// Parent box to hold all interior widgets
			SNew(SVerticalBox)

			// First slot for title, label, and help icon
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// ComboBox Label
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Bottom)
				.Padding(7.f)
				[
					ConstructLabelText(TEXT("Filter List:"))
				]

				// ComboBox Label
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Bottom)
				.Padding(5.f)
				[
					ConstructTitleText(TEXT("QUICK ASSET ACTIONS"))
				]

				// Help Icon
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(5.f)
				[
					ConstructHelpButton()
				]
			]

			// Second slot for filter options and refresh btn
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Filter dropdown
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.f, 0.f)
				[
					ConstructFilterComboBox()
				]

				// Manual Refresh btn
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.Padding(5.f, 0.f)
				[
					ConstructRefreshButton()
				]
			]

			// Third slot for list view
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.Padding(5.f)
			[
				SNew(SScrollBox)

				// List View
				+ SScrollBox::Slot()
				[
					ConstructAssetListView()
				]
			]


			// Fourth slot for helpful info to user 
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Number of assets in list
				+ SHorizontalBox::Slot()
				.Padding(10.f, 5.f)
				[
					ConstructTextForAssetCount()
				]

				// Selected folder path
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.FillWidth(12.f)
				.Padding(10.f, 5.f)
				[
					ConstructTextForSelectedFolderPath()
				]
			]

			// Fifth slot for bulk action buttons 
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				
				// Add Prefix to Selected
				+ SHorizontalBox::Slot()
				.FillWidth(5.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(AddPrefixSelected)
				]
				
				// Delete Selected 
				+ SHorizontalBox::Slot()
				.FillWidth(5.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(DeleteSelected)
				]

				// Duplicate Selected
				+ SHorizontalBox::Slot()
				.FillWidth(5.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(DuplicateSelected)
				]
				// Replace String for Selected
				+ SHorizontalBox::Slot()
				.FillWidth(5.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(ReplaceStringSelected)
				]
			]

		];

		// only need to filter data if we haven't already
		if (UnusedAssetsData.IsEmpty() || DuplicatedNameAssetsData.IsEmpty() || NoPrefixAssetsData.IsEmpty())
		{
			FilterAssetData();
		}
}

#pragma region TitleBar

TSharedRef<STextBlock> SAssetActionsTab::ConstructLabelText(const FString& LabelText)
/*
	Construct a STextBlock for the label above the ComboBox widget
*/
{
	FSlateFontInfo LabelFont = GetEmbossedFont();
	LabelFont.Size = 10;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(LabelText))
		.Font(LabelFont)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<STextBlock> SAssetActionsTab::ConstructTitleText(const FString& TitleText)
/*
	Construct a STextBlock for the Title Text at the top of the widget
*/
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TitleText))
		.Font(SharedTextFont)
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAssetActionsTab::ConstructHelpButton()
/*
	Construct a help button that opens a webpage in the browser that points to documentation for the tool
*/
{
	TSharedRef<SButton> ConstructedHelpButton =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(LOCTEXT("HelpDocumentationToolTip", "Go to documentation for Quick Asset Actions"))
		.ContentPadding(FMargin(5.f))
		
		.OnClicked(this, &SAssetActionsTab::OnHelpButtonClicked)
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FAppStyle::Get().GetBrush("StatusBar.HelpIcon"))
		];


	return ConstructedHelpButton;
}

FReply SAssetActionsTab::OnHelpButtonClicked()
/*
	When help button is clicked, launch the specified url
*/
{
	const FString& HelpUrl = "https://github.com/kjohnsoninfo/UnrealAssetActionsPlugin"; 
	FPlatformProcess::LaunchURL(*HelpUrl, NULL, NULL);
	
	return FReply::Handled();
}

#pragma endregion

#pragma region FilterSlot

TSharedRef<SComboBox<TSharedPtr<FString>>> SAssetActionsTab::ConstructFilterComboBox()
/* 
	Construct a combo box that contains and applies criteria to filter list view 
*/
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&FilterListItems)
		.OnGenerateWidget(this, &SAssetActionsTab::OnGenerateFilterItem) // must generate an SWidget
		.OnSelectionChanged(this, &SAssetActionsTab::OnFilterSelectionChanged)
		// Combo Box has a slot for text shown on construction; this is required
		[
			SAssignNew(ComboBoxDisplayedText, STextBlock)
			.Text(FText::FromString(ListAll)) // default list all
		];

	return ConstructedComboBox;
}

TSharedRef<SWidget> SAssetActionsTab::OnGenerateFilterItem(TSharedPtr<FString> FilterItem)
/*
	Construct a TextBlock for every filter item in the FilterListItems to display as options 
*/
{
	TSharedRef<STextBlock> ConstructedFilterTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(*FilterItem.Get()));

	return ConstructedFilterTextBlock;
}

// 
void SAssetActionsTab::OnFilterSelectionChanged(TSharedPtr<FString> SelectedFilter, ESelectInfo::Type InSelectInfo)
/* 
	Update ComboBox text and call appropriate filtering functions based on the user selected filter option 
	@note: OnSelectionChange requires a selectinfo parameter (see SComboBox.h from source)
*/
{
	const FString SelectedFilterText = *SelectedFilter.Get();

	// Set ComboBox text to selected filter
	ComboBoxDisplayedText->SetText(FText::FromString(SelectedFilterText));

	// Display all assets
	if (SelectedFilterText == ListAll)
	{
		DisplayedAssetsData = AllAssetsDataFromManager;
		RefreshWidget();
	}

	// Display only unused assets 
	else if (SelectedFilterText == ListUnused)
	{
		DisplayedAssetsData = UnusedAssetsData;
		RefreshWidget();
	}

	// Display assets with duplicate names 
	else if (SelectedFilterText == ListDuplicate)
	{
		// Filter items
		DisplayedAssetsData = DuplicatedNameAssetsData;
		RefreshWidget();
	}

	// Display assets with no prefixes 
	else if (SelectedFilterText == ListNoPrefix)
	{
		// Filter items
		DisplayedAssetsData = NoPrefixAssetsData;
		RefreshWidget();
	}
}


TSharedRef<SButton> SAssetActionsTab::ConstructRefreshButton()
/*
	Construct a refresh button that calls refresh asset list view fn
*/
{
	TSharedRef<SButton> ConstructedRefreshButton =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(LOCTEXT("RefreshBtnToolTip", "Refresh asset list view"))
		.ContentPadding(FMargin(5.f))

		.OnClicked(this, &SAssetActionsTab::OnRefreshButtonClicked)
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FAppStyle::Get().GetBrush("Icons.Refresh"))
		];


	return ConstructedRefreshButton;
}

FReply SAssetActionsTab::OnRefreshButtonClicked()
/*
	When refresh button is clicked, refresh asset list view and notify user
*/
{
	RefreshWidget();
	DebugHelper::NotificationPopup("Asset Actions List View Refreshed");
	return FReply::Handled();
}

#pragma endregion

#pragma region ListView

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAssetActionsTab::ConstructAssetListView()
/*
	Construct a SListView that generates rows for each asset found in the selected folder
*/
{
	ConstructedAssetListView =
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f) // height of each row
		.ListItemsSource(&DisplayedAssetsData) // pointer to array of source items
		.OnGenerateRow(this, &SAssetActionsTab::OnGenerateRowForListView) // create row for every asset found
		.OnMouseButtonDoubleClick(this, &SAssetActionsTab::OnRowDoubleClick)
		.HeaderRow
		(
			SNew(SHeaderRow)

			+ SHeaderRow::Column(AssetActionsColumns::Checkbox)
			.FillWidth(.3f)
			.VAlignHeader(EVerticalAlignment::VAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.SortMode(this, &SAssetActionsTab::GetSortModeForColumn, AssetActionsColumns::Checkbox)
			.OnSort(this, &SAssetActionsTab::OnSortModeChanged)
			[
				ConstructCheckBoxForHeaderRow()
			]

			+ SHeaderRow::Column(AssetActionsColumns::Class)
			.FillWidth(1.8f)
			.VAlignHeader(EVerticalAlignment::VAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.SortMode(this, &SAssetActionsTab::GetSortModeForColumn, AssetActionsColumns::Class)
			.OnSort(this, &SAssetActionsTab::OnSortModeChanged)
			[
				ConstructTextForHeaderRow(TEXT("Asset Type"))
			]

			+ SHeaderRow::Column(AssetActionsColumns::Name)
			.FillWidth(2.7f)
			.VAlignHeader(EVerticalAlignment::VAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.SortMode(this, &SAssetActionsTab::GetSortModeForColumn, AssetActionsColumns::Name)
			.OnSort(this, &SAssetActionsTab::OnSortModeChanged)
			[
				ConstructTextForHeaderRow(TEXT("Asset Name"))
			]

			+ SHeaderRow::Column(AssetActionsColumns::Path)
			.FillWidth(3.f)
			.VAlignHeader(EVerticalAlignment::VAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.SortMode(this, &SAssetActionsTab::GetSortModeForColumn, AssetActionsColumns::Path)
			.OnSort(this, &SAssetActionsTab::OnSortModeChanged)
			[
				ConstructTextForHeaderRow(TEXT("Asset Parent Folder"))
			]

			+ SHeaderRow::Column(AssetActionsColumns::RefCount)
			.FillWidth(.7f)
			.VAlignHeader(EVerticalAlignment::VAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.SortMode(this, &SAssetActionsTab::GetSortModeForColumn, AssetActionsColumns::RefCount)
			.OnSort(this, &SAssetActionsTab::OnSortModeChanged)
			[
				ConstructTextForHeaderRow(TEXT("# of Refs"))
			]

			+ SHeaderRow::Column(AssetActionsColumns::Rename)
			.FillWidth(.8f)
			.DefaultLabel(FText::FromString(TEXT("")))
		);

	DefaultSorting();

	return ConstructedAssetListView.ToSharedRef(); // convert to ref after construction
}

void SAssetActionsTab::OnRowDoubleClick(TSharedPtr<FAssetData> ClickedAssetData)
/*
	Navigate the content browser to the asset in the row doubleclicked on
*/
{
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();

	AssetActionsManager.SyncCBToClickedAsset(ClickedAssetData->GetObjectPathString());
}

TSharedRef<STextBlock> SAssetActionsTab::ConstructTextForHeaderRow(const FString& ColumnName)
/*
	Construct an STextBlock for each header row that displays the column name passed in
*/
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(ColumnName));

	// Fixes formatting for path column
	if (ColumnName == TEXT("Asset Parent Folder"))
	{
		ConstructedTextBlock->SetMargin(FMargin(30.f, 0.f, 0.f, 0.f));
	}

	return ConstructedTextBlock;
}

TSharedRef<SCheckBox> SAssetActionsTab::ConstructCheckBoxForHeaderRow()
{
	HeaderCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAssetActionsTab::OnHeaderCheckBoxStateChanged);

	return HeaderCheckBox.ToSharedRef();
}

void SAssetActionsTab::OnHeaderCheckBoxStateChanged(ECheckBoxState CheckBoxState)
/*
	Select all or deselect all checkboxes in the rows depending on the state of the header checkbox
*/
{
	switch (CheckBoxState)
	{
	// deselect all
	case ECheckBoxState::Unchecked:

		if (CheckBoxesArray.Num() == 0) return;

		for (TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
		{
			if (CheckBox->IsChecked())
			{
				CheckBox->ToggleCheckedState();
			}
		}

		break;

	// select all
	case ECheckBoxState::Checked:

		if (CheckBoxesArray.Num() == 0) return;

		for (TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
		{
			if (!CheckBox->IsChecked())
			{
				CheckBox->ToggleCheckedState();
			}
		}

		break;

	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

void SAssetActionsTab::DefaultSorting()
/*
	Sets default sort column and sort mode and sorts list view on widget spawn
*/
{
	SortByColumn = AssetActionsColumns::Name;
	SortMode = EColumnSortMode::Ascending;

	// A->Z primary sort = name; if equivalent, secondary sort = packagepath
	DisplayedAssetsData.Sort(
		[](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
		{
			return (A->AssetName.Compare(B->AssetName) < 0 ||
				((A->AssetName == B->AssetName) && (A->PackagePath.Compare(B->PackagePath) < 0)));
		});
}

EColumnSortMode::Type SAssetActionsTab::GetSortModeForColumn(const FName ColumnId) const
/*
	Get the sort mode of the column when user selects column to sort by.
	If user selected column does not equal column passed in from header row, set sort mode to none
*/
{
	return SortByColumn == ColumnId ? SortMode : EColumnSortMode::None;
}

void SAssetActionsTab::OnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type InSortMode)
/*
	Set sort column and sort mode to user selected option. Then refresh widget which updates the applied sort order. 
*/
{
	SortByColumn = ColumnId;
	SortMode = InSortMode;
	RefreshWidget();
}

void SAssetActionsTab::UpdateSorting()
/*
	Sort DisplayedAssetData by comparing the data within the user selected column; called in RefreshWidget
	Utilizes lambda fns to define sorting functions
*/
{

	// Sort by checked alphabetically and unchecked alphabetically
	if (SortByColumn == AssetActionsColumns::Checkbox)
	{
		if (SortMode == EColumnSortMode::Ascending)
		{
			TArray<TSharedPtr<FAssetData>> SortedCheckedAssets = CheckedAssets;
			TArray<TSharedPtr<FAssetData>> SortedUncheckedAssets = UncheckedAssets;

			SortedCheckedAssets.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
			{ return A->AssetName.Compare(B->AssetName) < 0; }); // A->Z

			SortedUncheckedAssets.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
			{ return A->AssetName.Compare(B->AssetName) < 0; }); // A->Z

			DisplayedAssetsData.Empty();
			DisplayedAssetsData.Append(CheckedAssets);
			DisplayedAssetsData.Append(UncheckedAssets);
		}
		else
		{
			TArray<TSharedPtr<FAssetData>> SortedCheckedAssets = CheckedAssets;
			TArray<TSharedPtr<FAssetData>> SortedUncheckedAssets = UncheckedAssets;

			SortedCheckedAssets.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return A->AssetName.Compare(B->AssetName) < 0; }); // A->Z

			SortedUncheckedAssets.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return A->AssetName.Compare(B->AssetName) < 0; }); // A->Z

			DisplayedAssetsData.Empty();
			DisplayedAssetsData.Append(UncheckedAssets);
			DisplayedAssetsData.Append(CheckedAssets);
		}
	}


	// Sort alphabetically by asset name
	if (SortByColumn == AssetActionsColumns::Name)
	{
		if (SortMode == EColumnSortMode::Ascending) 
		{
			// A->Z primary sort = name; if equivalent, secondary sort = packagepath
			DisplayedAssetsData.Sort(
				[](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ 
					return (A->AssetName.Compare(B->AssetName) < 0 ||
						((A->AssetName == B->AssetName) && (A->PackagePath.Compare(B->PackagePath) < 0)));
				}); 
		}
		else 
		{	
			// Z->A primary sort = name; if equivalent, secondary sort = packagepath
			DisplayedAssetsData.Sort(
				[](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{
					return (B->AssetName.Compare(A->AssetName) < 0 ||
						((B->AssetName == A->AssetName) && (B->PackagePath.Compare(A->PackagePath) < 0)));
				});
		}
	}

	// Sort alphabetically by asset class
	if (SortByColumn == AssetActionsColumns::Class)
	{
		if (SortMode == EColumnSortMode::Ascending)
		{
			DisplayedAssetsData.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return A->GetClass()->GetName().Compare(B->GetClass()->GetName()) < 0; }); // A->Z
		}
		else
		{
			DisplayedAssetsData.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return B->GetClass()->GetName().Compare(A->GetClass()->GetName()) < 0; }); // Z->A
		}
	}

	// Sort alphabetically by asset parent folder path
	if (SortByColumn == AssetActionsColumns::Path)
	{
		if (SortMode == EColumnSortMode::Ascending)
		{
			DisplayedAssetsData.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return A->PackagePath.Compare(B->PackagePath) < 0; }); // A->Z
		}
		else
		{
			DisplayedAssetsData.Sort([](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{ return B->PackagePath.Compare(A->PackagePath) < 0; }); // Z->A
		}
	}

	// Sort by asset reference count
	if (SortByColumn == AssetActionsColumns::RefCount)
	{
		FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();

		if (SortMode == EColumnSortMode::Ascending)
		{
			DisplayedAssetsData.Sort(
				[&](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{
					int32 CountA = AssetActionsManager.GetAssetReferencersCount(A);
					int32 CountB = AssetActionsManager.GetAssetReferencersCount(B);

					return CountA < CountB; // least->most
				});
		}
		else
		{
			DisplayedAssetsData.Sort(
				[&](const TSharedPtr<FAssetData>& A, const TSharedPtr<FAssetData>& B)
				{
					int32 CountA = AssetActionsManager.GetAssetReferencersCount(A);
					int32 CountB = AssetActionsManager.GetAssetReferencersCount(B);

					return CountB < CountA; // most->least
				});
		}
	}
}

#pragma endregion

#pragma region RowsInListView

TSharedRef<ITableRow> SAssetActionsTab::OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
/*
	Generate a STableRow for every asset and define child layout and widgets for each row
*/
{
	// check if AssetData is valid
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	// Get AssetData values
	const FString AssetName = AssetDataToDisplay->AssetName.ToString();
	const FString AssetClass = AssetDataToDisplay->GetClass()->GetName();
	const FString AssetParentFolder = AssetDataToDisplay->PackagePath.ToString();
	
	// Load manager module and call referencer count fn
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	const FString AssetRefCount = FString::FromInt(AssetActionsManager.GetAssetReferencersCount(AssetDataToDisplay));

	// return a ref to a table row to the OnGenerateRow fn
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> RowWidgetForListView =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.f))
		[
			// Parent container for row
			SNew(SHorizontalBox)

				// First slot for checkbox
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.FillWidth(.14f)
				[
					ConstructCheckBoxes(AssetDataToDisplay)
				]

				// Second slot for asset class
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.FillWidth(1.5f)
				[
					ConstructTextForRow(AssetClass)
				]

				// Third slot for asset name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.FillWidth(2.f)
				[
					ConstructTextForRow(AssetName)
				]

				// Fourth slot for parent folder path
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.FillWidth(2.5f)
				[
					ConstructTextForRow(AssetParentFolder)
				]

				// Fifth slot for asset referencer count
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.FillWidth(.2f)
				[
					ConstructTextForRow(AssetRefCount)
				]

				// Sixth slot for rename btn
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.FillWidth(.7f)
				.Padding(2.f, 2.f, 10.f, 2.f)
				[
					ConstructRenameButtonForRow(AssetDataToDisplay)
				]

		];

		return RowWidgetForListView;
}

TSharedRef<SCheckBox> SAssetActionsTab::ConstructCheckBoxes(const TSharedPtr<FAssetData>& AssetDataToDisplay)
/*
	Generate a SCheckBox for each row, add to CheckBoxArray, and set checkbox to previously checked state
*/
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAssetActionsTab::OnCheckBoxStateChanged, AssetDataToDisplay);

	CheckBoxesArray.Add(ConstructedCheckBox);

	// set checkboxes to previous state
	TMultiMap<FString, FString> CheckedAssetMap = GetCheckBoxAssetMap(CheckedAssets);
	TMultiMap<FString, FString> UncheckedAssetMap = GetCheckBoxAssetMap(UncheckedAssets);
	FString AssetName = AssetDataToDisplay->AssetName.ToString();
	FString AssetPath = AssetDataToDisplay->GetObjectPathString();

	if (CheckedAssetMap.FindPair(AssetName, AssetPath) != nullptr)
	{
		if (!ConstructedCheckBox->IsChecked())
		{
			ConstructedCheckBox->SetIsChecked(ECheckBoxState::Checked);
		}
	}

	if (UncheckedAssetMap.FindPair(AssetName, AssetPath) != nullptr)
	{
		if (ConstructedCheckBox->IsChecked())
		{
			ConstructedCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
	}

	return ConstructedCheckBox;
}

void SAssetActionsTab::OnCheckBoxStateChanged(ECheckBoxState CheckBoxState, TSharedPtr<FAssetData> ClickedAssetData)
/* 
	Add assets to checked and unchecked arrays and set header state based on row checkbox state
*/
{
	// Finding checkbox state based on both asset name and asset path
	TMultiMap<FString, FString> CheckedAssetMap = GetCheckBoxAssetMap(CheckedAssets);
	TMultiMap<FString, FString> UncheckedAssetMap = GetCheckBoxAssetMap(UncheckedAssets);
	FString AssetName = ClickedAssetData->AssetName.ToString();
	FString AssetPath = ClickedAssetData->GetObjectPathString();

	switch (CheckBoxState)
	{
	case ECheckBoxState::Unchecked:

		// if found in checked assets, remove asset
		
		if (CheckedAssetMap.FindPair(AssetName, AssetPath) != nullptr) 
		{
			TArray<TSharedPtr<FAssetData>> AssetsToRemove;

			for (TSharedPtr<FAssetData> Asset : CheckedAssets)
			{
				if (Asset->AssetName == AssetName && Asset->GetObjectPathString() == AssetPath)
				{
					AssetsToRemove.AddUnique(Asset);
				}
			}

			for (TSharedPtr<FAssetData> AssetToRemove : AssetsToRemove)
			{
				CheckedAssets.Remove(AssetToRemove);
			}
		}

		// if not found in unchecked assets, add asset
		if (UncheckedAssetMap.FindPair(AssetName, AssetPath) == nullptr)
		{
			UncheckedAssets.AddUnique(ClickedAssetData);
		}

		// if row checkboxes have both states, set headercheckbox to undetermined
		if (UncheckedAssets.Num() > 0 && CheckedAssets.Num() > 0)
		{
			HeaderCheckBox->SetIsChecked(ECheckBoxState::Undetermined);
		}

		// if row checkboxes are all unchecked, set headercheckbox to unchecked
		if (CheckedAssets.Num() == 0)
		{
			HeaderCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}

		break;

	case ECheckBoxState::Checked:

		// if found in unchecked assets, remove asset
		if (UncheckedAssetMap.FindPair(AssetName, AssetPath) != nullptr)
		{
			TArray<TSharedPtr<FAssetData>> AssetsToRemove;

			for (TSharedPtr<FAssetData> Asset : UncheckedAssets)
			{
				if (Asset->AssetName == AssetName && Asset->GetObjectPathString() == AssetPath)
				{
					AssetsToRemove.AddUnique(Asset);
				}
			}

			for (TSharedPtr<FAssetData> Asset : AssetsToRemove)
			{
				UncheckedAssets.Remove(Asset);
			}
		}

		// if not found in checked assets, add asset
		if (CheckedAssetMap.FindPair(AssetName, AssetPath) == nullptr)
		{
			CheckedAssets.AddUnique(ClickedAssetData);
		}

		// if row checkboxes have both states, set headercheckbox to undetermined
		if (UncheckedAssets.Num() > 0 && CheckedAssets.Num() > 0)
		{
			HeaderCheckBox->SetIsChecked(ECheckBoxState::Undetermined);
		}

		// if row checkboxes are all checked, set headercheckbox to checked
		if (UncheckedAssets.Num() == 0)
		{
			HeaderCheckBox->SetIsChecked(ECheckBoxState::Checked);
		}

		break;

	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

TSharedRef<STextBlock> SAssetActionsTab::ConstructTextForRow(const FString& RowText)
/*
	Common function to construct any textblocks for rows in the list view
*/
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(RowText))
		.Font(SharedTextFont)
		.ColorAndOpacity(FColor::White)
		.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
		.AutoWrapText(true);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAssetActionsTab::ConstructRenameButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay)
/*
	Construct rename button for each row in the list view
*/
{
	TSharedRef<SButton> ConstructedRenameButton =
		SNew(SButton)
		.OnClicked(this, &SAssetActionsTab::OnRenameButtonClicked, AssetDataToDisplay);

	ConstructedRenameButton->SetContent(ConstructTextForRow(TEXT("Rename")));

	return ConstructedRenameButton;
}

FReply SAssetActionsTab::OnRenameButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
/*
	Rename a single asset by passing in AssetDataToDisplay for the clicked row
*/
{
	// Spawn modal window/dialog to get user inputs
	TSharedRef<SWindow> RenameAssetWindow =
		SNew(SWindow)
		.Title(LOCTEXT("RenameAssetWindowTitle", "Rename Asset"))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SRenameAssetDialog> RenameAssetDialog =
		SNew(SRenameAssetDialog)
		.AssetData(ClickedAssetData);

	RenameAssetWindow->SetContent(SNew(SBox)
		.MinDesiredWidth(320.0f)
		[
			RenameAssetDialog
		]);

	TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());

	FSlateApplication::Get().AddModalWindow(RenameAssetWindow, CurrentWindow);

	// Rename asset based on user input passed in by dialog
	FString NewName = RenameAssetDialog->NewName;
	RenameAsset(NewName, ClickedAssetData);

	return FReply::Handled();
}

void SAssetActionsTab::RenameAsset(const FString& NewName, const TSharedPtr<FAssetData>& AssetToRename)
/* 
	Call manager fn to rename clicked asset and if asset is successfully renamed, refresh widget
*/
{
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();

	if (NewName.IsEmpty()) { return; }

	bool bAssetRenamed = AssetActionsManager.RenameAssetInList(NewName, AssetToRename);

	if (bAssetRenamed)
	{
		RefreshWidget();
	}
}

#pragma endregion

#pragma region ButtonSlot

TSharedRef<SButton> SAssetActionsTab::ConstructButtonForSlot(const FString& ButtonName)
/*
	Common function to construct buttons in the button slot
*/
{
	TSharedRef<SButton> ConstructedSlotButton =
		SNew(SButton)
		.ContentPadding(FMargin(5.f));

	// Set content through a common fn
	ConstructedSlotButton->SetContent(ConstructTextForButtonSlot(ButtonName));

	// Create lamda function to pass in on clicked function based on button name
	// This allows a single function to construct the button but on clicked action is different
	ConstructedSlotButton->SetOnClicked(FOnClicked::CreateLambda([this, ButtonName]()
		{
			AssignButtonClickFns(ButtonName);
			return FReply::Handled();
		}
	));

	return ConstructedSlotButton;
}

TSharedRef<STextBlock> SAssetActionsTab::ConstructTextForButtonSlot(const FString& ButtonName)
{
	TSharedRef<STextBlock> ConstructedTextForButtonSlot =
		SNew(STextBlock)
		.Text(FText::FromString(ButtonName))
		.Font(SharedTextFont)
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextForButtonSlot;
}

void SAssetActionsTab::AssignButtonClickFns(const FString& ButtonName)
/*
	Assign the different onClicked fns to each button based on button name
*/
{
	if (ButtonName == AddPrefixSelected)
	{
		OnAddPrefixButtonClicked();
	}
	else if (ButtonName == DeleteSelected)
	{
		OnDeleteSelectedButtonClicked();
	}
	else if (ButtonName == DuplicateSelected)
	{
		OnDuplicateSelectedButtonClicked();
	}
	else if (ButtonName == ReplaceStringSelected)
	{
		OnReplaceStringButtonClicked();
	}
}

FReply SAssetActionsTab::OnAddPrefixButtonClicked()
/*
	Attempts to add prefixes to all assets in CheckedAssets and if successful, refreshes widget
*/
{
	if (CheckedAssets.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
	}

	// Call add prefix fn from manager module passing in the checked data
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	bool bPrefixesAdded = AssetActionsManager.AddPrefixesToAssetsInList(CheckedAssets);

	if (bPrefixesAdded)
	{
		RefreshWidget();
	}

	return FReply::Handled();
}

FReply SAssetActionsTab::OnDeleteSelectedButtonClicked()
/*
	Attempts to delete all assets in CheckedAssets and if successful, refreshes widget
*/
{
	if (CheckedAssets.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
	}
	
	// Convert array of ptr to array of FAssetData for delete fn
	TArray<FAssetData> AssetsToDelete;
		
	for (const TSharedPtr<FAssetData>& AssetData : CheckedAssets)
	{
		AssetsToDelete.Add(*AssetData.Get());
	}

	// Call delete fn from manager module passing in the checked data
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	bool bAssetDeleted = AssetActionsManager.DeleteAssetsInList(AssetsToDelete);

	// Remove from list view if asset was deleted and refresh widget
	if (bAssetDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedAsset : CheckedAssets)
		{
			EnsureAssetDeletionFromLists(DeletedAsset);
		}

		RefreshWidget();
	}

	return FReply::Handled();
}

FReply SAssetActionsTab::OnDuplicateSelectedButtonClicked()
/*
	Attempts to duplicate all assets in CheckedAssets and if successful, refreshes widget
*/
{
	if (CheckedAssets.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
	}

	// Spawn modal dialog to get user input
	int32 NumOfDuplicates = GetUserNumberForDuplicates();

	// Call duplicate fn from manager module passing in the user input and checked data
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	bool bAssetsDuplicated = AssetActionsManager.DuplicateAssetsInList(NumOfDuplicates, CheckedAssets);

	if (bAssetsDuplicated)
	{
		RefreshWidget();
	}

	return FReply::Handled();
}

int32 SAssetActionsTab::GetUserNumberForDuplicates()
/*
	Spawns a custom dialog that asks for user numeric entry 
*/
{
	int32 NumOfDuplicates = 1;
	bool bEnterPressed = false;

	TSharedRef<SCustomDialog> DuplicateAssetsDialog =
		SNew(SCustomDialog)
		.Title(FText::FromString(DuplicateSelected))
		.HAlignContent(HAlign_Center)
		.HAlignButtonBox(HAlign_Fill)
		.ButtonAreaPadding(5.f)
		.Content()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(5.f)
			.AutoWidth()
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Number of duplicates: ")))
				.Font(SharedTextFont)
				.ColorAndOpacity(FColor::White)
			]

			+ SHorizontalBox::Slot()
			.Padding(2.f)
			.FillWidth(25.f)
			[
				SNew(SNumericEntryBox<int32>)
					.AllowSpin(false)
					.Font(SharedTextFont)
					.MinValue(1)
					.MaxValue(25)
					.Value(1)
					.Value_Lambda([&] { return TOptional<int32>(NumOfDuplicates); })
					.OnValueChanged_Lambda([&](int32 InValue) { NumOfDuplicates = InValue; })
					.OnValueCommitted_Lambda(
						[&](int32 InValue, ETextCommit::Type CommitInfo) 
						{
							// allow user to press keyboard enter as "OK"
							if (CommitInfo == ETextCommit::OnEnter)
							{
								NumOfDuplicates = InValue;
								DuplicateAssetsDialog->RequestDestroyWindow();
								bEnterPressed = true;
							}
							else
							{
								NumOfDuplicates = InValue;
							}
						})
			]
		]
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel")),
			SCustomDialog::FButton(LOCTEXT("OK", "OK"))
			});;

	const int ButtonPressed = DuplicateAssetsDialog->ShowModal();

	if (ButtonPressed == 1 || bEnterPressed)
	{
		return NumOfDuplicates;
	}

	// user entry was cancelled
	else
	{
		return 0;
	}
}

FReply SAssetActionsTab::OnReplaceStringButtonClicked()
/*
	Attempts to replace string in all asset names in CheckedAssets and if successful, refreshes widget
*/
{
	if (CheckedAssets.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
	}

	// Spawn a dialog for user input
	TSharedRef<SWindow> ReplaceStringWindow =
		SNew(SWindow)
		.Title(LOCTEXT("ReplaceStringWindowTitle", "Replace String in Assets Names"))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SReplaceStringDialog> ReplaceStringDialog =
		SNew(SReplaceStringDialog);

	ReplaceStringWindow->SetContent(SNew(SBox)
		.MinDesiredWidth(320.0f)
		[
			ReplaceStringDialog
		]);

	TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());

	FSlateApplication::Get().AddModalWindow(ReplaceStringWindow, CurrentWindow);

	FString OldString = ReplaceStringDialog->OldString;
	FString NewString = ReplaceStringDialog->NewString;

	if (OldString.IsEmpty() || NewString.IsEmpty())
	{
		return FReply::Handled();
	}

	// Call replace string fn from manager module passing in the user input and checked data
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	bool bStringReplaced = AssetActionsManager.ReplaceString(OldString, NewString, CheckedAssets);

	if (bStringReplaced)
	{
		RefreshWidget();
	}

	return FReply::Handled();
}

#pragma endregion

#pragma region HelpfulInfoSlot

TSharedRef<SRichTextBlock> SAssetActionsTab::ConstructTextForAssetCount()
/*
	Construct a textblock that can be dynamically refreshed that keeps track of displayed asset count
*/
{
	const FTextBlockStyle TextBlockStyle = 
		FTextBlockStyle().SetFont(SharedTextFont).SetColorAndOpacity(FColor::White);

	ConstructedAssetCountTextBlock =
		SNew(SRichTextBlock);
		
	ConstructedAssetCountTextBlock->SetText(FText::FromString(AssetCountMsg));
	ConstructedAssetCountTextBlock->SetTextStyle(TextBlockStyle);

	return ConstructedAssetCountTextBlock.ToSharedRef();
}

TSharedRef<STextBlock> SAssetActionsTab::ConstructTextForSelectedFolderPath()
{
	FString SelectedFolderPathsText;
	FString SelectedFolderDisplayedText;

	// when only one folder is selected, set text equal to only element
	if (SelectedFoldersPaths.Num() == 1)
	{
		SelectedFolderPathsText = SelectedFoldersPaths[0];
		SelectedFolderDisplayedText = SelectedFolderPathsText;
	}

	// else append each folder path with a + separator
	else if (SelectedFoldersPaths.Num() > 1)
	{
		for (int i = 0; i < SelectedFoldersPaths.Num(); ++i)
		{
			// if last element in list, don't add + seperator
			if (i == SelectedFoldersPaths.Num() - 1)
			{
				SelectedFolderPathsText.Append(SelectedFoldersPaths[i]);
			}

			else
			{
				SelectedFolderPathsText.Append(SelectedFoldersPaths[i] + TEXT(" + "));
			}
		}

		if (SelectedFoldersPaths.Num() > 4)
		{
			SelectedFolderDisplayedText = 
				TEXT("Too many folders selected to display. Refer to Content Browser");
		}

		SelectedFolderDisplayedText = SelectedFolderPathsText;
	}

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Selected folder path: ") + SelectedFolderDisplayedText))
		.Font(SharedTextFont)
		.ColorAndOpacity(FColor::White)
		.AutoWrapText(true);

	return ConstructedTextBlock;
}

#pragma endregion

#pragma region HelperFunctions

void SAssetActionsTab::RefreshWidget()
/*
	Refresh asset data and source items to ensure AssetListView and AssetCount is always up to date
*/
{
	// Call fix up redirectors fn from manager module
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();
	AssetActionsManager.FixUpRedirectors();

	// Refresh source items to pick up changes
	AllAssetsDataFromManager = AssetActionsManager.GetAllAssetDataUnderSelectedFolder();

	FilterAssetData();
	
	if (ComboBoxDisplayedText->GetText().ToString() == ListAll)
	{
		DisplayedAssetsData = AllAssetsDataFromManager;
	}

	else if (ComboBoxDisplayedText->GetText().ToString() == ListUnused)
	{
		DisplayedAssetsData = UnusedAssetsData;
	}

	else if (ComboBoxDisplayedText->GetText().ToString() == ListDuplicate)
	{
		DisplayedAssetsData = DuplicatedNameAssetsData;
	}

	else if (ComboBoxDisplayedText->GetText().ToString() == ListNoPrefix)
	{
		DisplayedAssetsData = NoPrefixAssetsData;
	}

	// Refresh sorting
	UpdateSorting();

	// Refresh Asset List View
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}

	// Refresh Asset Count Text
	AssetCountMsg = GetAssetCountMsg();

	if (ConstructedAssetCountTextBlock.IsValid())
	{
		ConstructedAssetCountTextBlock->SetText(FText::FromString(AssetCountMsg));
		ConstructedAssetCountTextBlock->Refresh();
	}
}

void SAssetActionsTab::FilterAssetData()
/*
	Call all the filter fns from the manager module and sets the appropriate asset data variable
*/
{
	FAssetActionsManagerModule& AssetActionsManager = LoadManagerModule();

	UnusedAssetsData = AssetActionsManager.FilterForUnusedAssetData(AllAssetsDataFromManager);
	DuplicatedNameAssetsData = AssetActionsManager.FilterForDuplicateNameData(AllAssetsDataFromManager);
	NoPrefixAssetsData = AssetActionsManager.FilterForNoPrefixData(AllAssetsDataFromManager);
}

void SAssetActionsTab::EnsureAssetDeletionFromLists(const TSharedPtr<FAssetData>& AssetDataToDelete)
/*
	Remove deleted assets from all asset data arrays if they contain the deleted asset
*/
{
	if (DisplayedAssetsData.Contains(AssetDataToDelete))
	{
		DisplayedAssetsData.Remove(AssetDataToDelete);
	}

	if (AllAssetsDataFromManager.Contains(AssetDataToDelete))
	{
		AllAssetsDataFromManager.Remove(AssetDataToDelete);
	}

	if (UnusedAssetsData.Contains(AssetDataToDelete))
	{
		UnusedAssetsData.Remove(AssetDataToDelete);
	}

	if (DuplicatedNameAssetsData.Contains(AssetDataToDelete))
	{
		DuplicatedNameAssetsData.Remove(AssetDataToDelete);
	}

	if (NoPrefixAssetsData.Contains(AssetDataToDelete))
	{
		NoPrefixAssetsData.Remove(AssetDataToDelete);
	}
}

TMultiMap<FString, FString> SAssetActionsTab::GetCheckBoxAssetMap(const TArray<TSharedPtr<FAssetData>> CheckBoxStateArray)
/*
	Returns a multimap of asset names to asset paths
	This is important for setting a checkbox state to its previous state
*/
{
	TMultiMap<FString, FString> AssetMap;

	for (TSharedPtr<FAssetData> Asset : CheckBoxStateArray)
	{
		AssetMap.Add(Asset->AssetName.ToString(), Asset->GetObjectPathString());
	}

	return AssetMap;
}

void SAssetActionsTab::ClearCheckedStates()
{
	CheckBoxesArray.Empty();
	CheckedAssets.Empty();
	UncheckedAssets.Empty();
}

#pragma endregion


