// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "DebugHelper.h"
#include "AssetActionsManager.h"

#define LOCTEXT_NAMESPACE "SAdvancedDeletionTab"
#define DeleteSelected TEXT("Delete Selected")
#define SelectAll TEXT("Select All")
#define DeselectAll TEXT("Deselect All")


void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
/*
	Widget constructor that initializes all variables and Slate components
*/
{
	bCanSupportFocus = true;
	AssetsDataFromManager = InArgs._AssetsDataFromManager; // set widget data array to data passed in from manager
	SharedTextFont = GetEmbossedFont();
	SharedTextFont.Size = 12;

	// Ensure clean slate when constructed
	CheckBoxesArray.Empty();
	CheckedAssetsToDelete.Empty();

	ChildSlot
		[
			// Parent box to hold all interior widgets
			SNew(SVerticalBox)

			// First slot for title and help icon
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				[
					ConstructTitleTextForTab(TEXT("ADVANCED DELETION"))
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

			// Second slot for search and filter options
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Search Bar
				+ SHorizontalBox::Slot()

				// Filter
				+ SHorizontalBox::Slot()
			]

			// Third slot for list view
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
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

				// Current folder path
				+ SHorizontalBox::Slot()
			]

			// Fifth slot for quick buttons 
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Delete Selected 
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(DeleteSelected)
				]

				// Select All
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(SelectAll)
				]
				// Deselect All
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructButtonForSlot(DeselectAll)
				]
			]

		];
}

#pragma region TitleBar

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTitleTextForTab(const FString& TitleText)
/*
	Construct a STextBlock for the Title Text at the top of the widget
*/
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TitleText))
		.Font(SharedTextFont)
		.Justification(ETextJustify::Left)
		.Margin(FMargin(10.f))
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructHelpButton()
/*
	Construct a help button that opens a webpage in the browser that points to documentation for the widget
*/
{
	TSharedRef<SButton> ConstructedHelpButton =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(LOCTEXT("HelpDocumentationToolTip", "Go to documentation for Advanced Deletion"))
		.ContentPadding(FMargin(5.f))
		
		.OnClicked(this, &SAdvancedDeletionTab::OnHelpButtonClicked)
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FAppStyle::Get().GetBrush("StatusBar.HelpIcon"))
		];


	return ConstructedHelpButton;
}

FReply SAdvancedDeletionTab::OnHelpButtonClicked()
/*
	When help button is clicked, launch the specified url
*/
{
	const FString& HelpUrl = "https://www.unrealengine.com/en-US"; 
	FPlatformProcess::LaunchURL(*HelpUrl, NULL, NULL);
	
	return FReply::Handled();
}

#pragma endregion

#pragma region ListView

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeletionTab::ConstructAssetListView()
/*
	Construct a SListView that generates rows for each asset found in the selected folder
*/
{
	ConstructedAssetListView =
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f) // height of each row
		.ListItemsSource(&AssetsDataFromManager) // pointer to array of source items
		.OnGenerateRow(this, &SAdvancedDeletionTab::OnGenerateRowForListView) // create row for every asset found
		.HeaderRow
		(
			SNew(SHeaderRow)

			+ SHeaderRow::Column("CheckBox")
			+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString(TEXT("Asset Name")))
			+ SHeaderRow::Column("Class").DefaultLabel(FText::FromString(TEXT("Asset Class")))
			+ SHeaderRow::Column("Path").DefaultLabel(FText::FromString(TEXT("Asset Parent Folder")))
			+ SHeaderRow::Column("Delete").DefaultLabel(FText::FromString(TEXT(""))).FillWidth(.375f)
		);

	return ConstructedAssetListView.ToSharedRef(); // convert to ref after construction
}

void SAdvancedDeletionTab::RefreshAssetListView()
/*
	Call RebuildList to ensure AssetListView is always up to date
*/
{
	// ensure clean slate when refreshed
	CheckBoxesArray.Empty();
	CheckedAssetsToDelete.Empty();

	// since ptr can be null, check if valid before refresh
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma endregion

#pragma region RowsInListView

TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
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

	// return a ref to a table row to the OnGenerateRow fn
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> RowWidgetForListView =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(5.f))
		[
			// Parent container for row
			SNew(SHorizontalBox)

				// First slot for checkbox
				+ SHorizontalBox::Slot()
				[
					ConstructCheckBoxes(AssetDataToDisplay)
				]

				// Second slot for asset name
				+ SHorizontalBox::Slot()
				[
					ConstructTextForRow(AssetName)
				]
				// Third slot for asset class
				+ SHorizontalBox::Slot()
				[
					ConstructTextForRow(AssetClass)
				]

				// Fourth slot for parent folder path
				+ SHorizontalBox::Slot()
				[
					ConstructTextForRow(AssetParentFolder)
				]

				// Fifth slot for single deletion
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.FillWidth(.5f)
				[
					ConstructDeleteButtonForRow(AssetDataToDisplay)
				]

		];

		return RowWidgetForListView;
}

TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBoxes(const TSharedPtr<FAssetData>& AssetDataToDisplay)
/*
	Generate a SCheckBox for each row and add to CheckBoxesArray
*/
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvancedDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay);

	CheckBoxesArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState CheckBoxState, TSharedPtr<FAssetData> ClickedAssetData)
{
	// Add or remove assets from delete array based on checkbox state
	switch (CheckBoxState)
	{
	case ECheckBoxState::Unchecked:

		if (CheckedAssetsToDelete.Contains(ClickedAssetData))
		{
			CheckedAssetsToDelete.Remove(ClickedAssetData);
		}

		break;
	case ECheckBoxState::Checked:

		CheckedAssetsToDelete.AddUnique(ClickedAssetData);

		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForRow(const FString& RowText)
/*
	Common function to construct any textblocks for rows in the list view
*/
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(RowText))
		.Font(SharedTextFont)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeleteButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay)
/*
	Construct delete button that deletes a single asset for each row in the list view
*/
{
	TSharedRef<SButton> ConstructedDeleteButton = 
		SNew(SButton)
		.OnClicked(this, &SAdvancedDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	ConstructedDeleteButton->SetContent(ConstructTextForRow(TEXT("Delete")));

	return ConstructedDeleteButton;
}

FReply SAdvancedDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData) 
/*
	Delete a single asset by passing in AssetDataToDisplay for the clicked row
*/
{
	// Add single asset to array since ObjectTools::Delete expects an array
	TArray<FAssetData> AssetToDelete;
	AssetToDelete.Add(*ClickedAssetData.Get()); // deref from Ptr

	// Load manager module
	FAssetActionsManagerModule& AssetActionsManager =
	FModuleManager::LoadModuleChecked<FAssetActionsManagerModule>(TEXT("AssetActionsManager"));

	// Call delete fn from manager module passing in the clicked data
	bool bAssetDeleted = AssetActionsManager.DeleteAssetsInList(AssetToDelete);

	// Remove from list view if asset was deleted
	if (bAssetDeleted)
	{
		if (AssetsDataFromManager.Contains(ClickedAssetData))
		{
			AssetsDataFromManager.Remove(ClickedAssetData);
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

#pragma endregion

#pragma region ButtonSlot

TSharedRef<SButton> SAdvancedDeletionTab::ConstructButtonForSlot(const FString& ButtonName)
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

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForButtonSlot(const FString& ButtonName)
{
	TSharedRef<STextBlock> ConstructedTextForButtonSlot =
		SNew(STextBlock)
		.Text(FText::FromString(ButtonName))
		.Font(SharedTextFont)
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextForButtonSlot;
}

void SAdvancedDeletionTab::AssignButtonClickFns(const FString& ButtonName)
/*
	Assign the different onClicked fns to each button based on button name
*/
{
	if (ButtonName == DeleteSelected)
	{
		OnDeleteSelectedButtonClicked();
	}
	else if (ButtonName == SelectAll)
	{
		OnSelectAllButtonClicked();
	}
	else if (ButtonName == DeselectAll)
	{
		OnDeselectAllButtonClicked();
	}
}

FReply SAdvancedDeletionTab::OnDeleteSelectedButtonClicked()
/*
	Deletes all assets in CheckedAssetsToDeleteArray
*/
{
	if (CheckedAssetsToDelete.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets selected."));
		return FReply::Handled();
	}
	
	// Convert array of ptr to array of FAssetData for delete fn
	TArray<FAssetData> AssetsToDelete;
		
	for (const TSharedPtr<FAssetData>& AssetData : CheckedAssetsToDelete)
	{
		AssetsToDelete.Add(*AssetData.Get());
	}

	// Load manager module
	FAssetActionsManagerModule& AssetActionsManager =
		FModuleManager::LoadModuleChecked<FAssetActionsManagerModule>(TEXT("AssetActionsManager"));

	// Call delete fn from manager module passing in the checked data
	bool bAssetDeleted = AssetActionsManager.DeleteAssetsInList(AssetsToDelete);

	// Remove from list view if asset was deleted
	if (bAssetDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedAsset : CheckedAssetsToDelete)
		{
			if (AssetsDataFromManager.Contains(DeletedAsset))
			{
				AssetsDataFromManager.Remove(DeletedAsset);
			}
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnSelectAllButtonClicked()
/*
	Check state of checkboxes in CheckBoxesArray and toggle to checked if not already checked
*/
{
	if (CheckBoxesArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnDeselectAllButtonClicked()
/*
	Check state of checkboxes in CheckBoxesArray and toggle to unchecked if already checked
*/
{
	if (CheckBoxesArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

#pragma endregion
