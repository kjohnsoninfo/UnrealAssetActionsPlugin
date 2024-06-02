// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "DebugHelper.h"
#include "AssetActionsManager.h"

#define LOCTEXT_NAMESPACE "SAdvancedDeletionTab"

void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	AssetsDataFromManager = InArgs._AssetsDataFromManager; // set widget data array to data passed in from manager

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

				// Select All
				+ SHorizontalBox::Slot()

				// Deselect All
				+ SHorizontalBox::Slot()
			]

		];
}

#pragma region TitleBar

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTitleTextForTab(const FString& TitleText)
{
	FSlateFontInfo TextFont = GetEmbossedFont();
	TextFont.Size = 10;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TitleText))
		.Font(TextFont)
		.Justification(ETextJustify::Left)
		.Margin(FMargin(10.f))
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructHelpButton()
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
{
	const FString& HelpUrl = "https://www.unrealengine.com/en-US";
	FPlatformProcess::LaunchURL(*HelpUrl, NULL, NULL);
	
	return FReply::Handled();
}

#pragma endregion

#pragma region ListView

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeletionTab::ConstructAssetListView()
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
{
	// since ptr can be null, check if valid before refresh
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma endregion

#pragma region RowsInListView

TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForListView(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
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

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForRow(const FString& RowText)
{
	FSlateFontInfo TextFont = GetEmbossedFont();
	TextFont.Size = 10;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(RowText))
		.Font(TextFont)
		.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeleteButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedDeleteButton = 
		SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SAdvancedDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedDeleteButton;
}

FReply SAdvancedDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData) // ClickedData = AssetDataToDisplay for the selected row
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
