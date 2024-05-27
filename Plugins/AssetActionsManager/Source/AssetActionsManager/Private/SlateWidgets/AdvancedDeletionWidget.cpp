// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"

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

				// Help Icon
				+ SHorizontalBox::Slot()
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
