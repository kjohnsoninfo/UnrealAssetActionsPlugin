// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateWidgets/RenameAssetDialog.h"

#include "DebugHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "RenameAsset"

void SRenameAssetDialog::Construct(const FArguments& InArgs)
{
	AssetData = InArgs._AssetData;

	FSlateFontInfo DialogFont = FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont"));
	DialogFont.Size = 10;

	ChildSlot
		[
			SNew(SBox)
				.Padding(InArgs._Padding)
				[
					SNew(SGridPanel)
						.FillColumn(1, 1.0)

						// Current name display
						+ SGridPanel::Slot(0, 0)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Font(DialogFont)
								.Text(LOCTEXT("CurrentName", "Current Name:"))
						]

						+ SGridPanel::Slot(1, 0)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SNew(STextBlock)
								.Font(DialogFont)
								.Text(FText::FromName(AssetData->AssetName))
						]

						// New name controls
						+ SGridPanel::Slot(0, 1)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Font(DialogFont)
								.Text(LOCTEXT("NewName", "New Name:"))
						]

						+ SGridPanel::Slot(1, 1)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(NewNameTextBox, SEditableTextBox)
								.Font(DialogFont)
								.Text(FText::FromName(AssetData->AssetName))
								.OnTextCommitted(this, &SRenameAssetDialog::OnRenameTextCommitted)
						]

						// Dialog controls
						+ SGridPanel::Slot(0, 2)
						.ColumnSpan(2)
						.HAlign(HAlign_Right)
						.Padding(FMargin(0, 16))
						[
							SNew(SHorizontalBox)

								// Rename
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 8.0f, 0)
								[
									SNew(SButton)
										.IsFocusable(false)
										.OnClicked(this, &SRenameAssetDialog::OnRenameClicked)
										[
											SNew(STextBlock)
												.Text(LOCTEXT("RenameButtonText", "Rename"))
										]
								]

								// Cancel
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SButton)
										.IsFocusable(false)
										.OnClicked(this, &SRenameAssetDialog::OnCancelClicked)
										[
											SNew(STextBlock)
												.Text(LOCTEXT("CancelRenameButtonText", "Cancel"))
										]
								]
						]
				]
		];
}

FString SRenameAssetDialog::RenameAndClose()
{
	const FString OldName = AssetData->AssetName.ToString();
	const FString NewAssetName = NewNameTextBox->GetText().ToString();

	if (NewAssetName.IsEmpty())
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("Please enter a valid name."));
		return "";
	}

	else if (NewAssetName == OldName)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("Please enter a new name."));
		return "";
	}

	else
	{
		CloseContainingWindow();
		return NewAssetName;
	}
}

void SRenameAssetDialog::OnRenameTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter)
	{
		NewName = RenameAndClose();
	}
}

FReply SRenameAssetDialog::OnRenameClicked()
{
	NewName = RenameAndClose();

	return FReply::Handled();
}

FReply SRenameAssetDialog::OnCancelClicked()
{
	CloseContainingWindow();

	return FReply::Handled();
}

void SRenameAssetDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if (CurrentWindow.IsValid())
	{
		CurrentWindow->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE
