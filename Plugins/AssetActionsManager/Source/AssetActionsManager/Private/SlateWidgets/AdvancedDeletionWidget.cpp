// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"

#define LOCTEXT_NAMESPACE "SAdvancedDeletionTab"

void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

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
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// List View
				+ SHorizontalBox::Slot()

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

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTitleTextForTab(const FString& TextContent)
{
	FSlateFontInfo TextFont = GetEmbossedFont();
	TextFont.Size = 10;

	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
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
