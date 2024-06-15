// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateWidgets/ReplaceStringDialog.h"

#include "DebugHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "ReplaceString"

void SReplaceStringDialog::Construct(const FArguments& InArgs)
{
	FSlateFontInfo DialogFont = FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont"));
	DialogFont.Size = 10;

	ChildSlot
		[
			SNew(SBox)
				.Padding(InArgs._Padding)
				[
					SNew(SGridPanel)
						.FillColumn(1, 1.0)

						// Info Text
						+ SGridPanel::Slot(0, 0)
						.ColumnSpan(2)
						.Padding(10)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Replacing a string is case sensitive!")))
								.Font(DialogFont)
						]

						// String to replace entry
						+ SGridPanel::Slot(0, 1)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Font(DialogFont)
								.Text(LOCTEXT("StringToReplace", "String To Replace:"))
						]

						+ SGridPanel::Slot(1, 1)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(OldStringTextBox, SEditableTextBox)
								.Font(DialogFont)
								.OnTextCommitted(this, &SReplaceStringDialog::OnReplaceTextCommitted)
						]

						// New string entry 
						+ SGridPanel::Slot(0, 2)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Font(DialogFont)
								.Text(LOCTEXT("NewName", "New String:"))
						]

						+ SGridPanel::Slot(1, 2)
						.Padding(2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(NewStringTextBox, SEditableTextBox)
								.Font(DialogFont)
								.OnTextCommitted(this, &SReplaceStringDialog::OnReplaceTextCommitted)
						]
						// Dialog controls
						+ SGridPanel::Slot(0, 3)
						.ColumnSpan(2)
						.HAlign(HAlign_Right)
						.Padding(FMargin(0, 16))
						[
							SNew(SHorizontalBox)

								// Replace
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 8.0f, 0)
								[
									SNew(SButton)
										.IsFocusable(false)
										.OnClicked(this, &SReplaceStringDialog::OnReplaceClicked)
										[
											SNew(STextBlock)
												.Text(LOCTEXT("ReplaceButtonText", "Replace"))
										]
								]

								// Cancel
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SButton)
										.IsFocusable(false)
										.OnClicked(this, &SReplaceStringDialog::OnCancelClicked)
										[
											SNew(STextBlock)
												.Text(LOCTEXT("CancelButtonText", "Cancel"))
										]
								]
						]
				]
		];
}

void SReplaceStringDialog::ReplaceAndClose()
{
	FString OldStringInput = OldStringTextBox->GetText().ToString();
	FString NewStringInput = NewStringTextBox->GetText().ToString();

	if (OldStringInput.IsEmpty())
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("Old string entry is empty."));
	}

	else if (NewStringInput.IsEmpty())
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("New string entry is empty."));
	}

	else
	{
		OldString = OldStringInput;
		NewString = NewStringInput;

		CloseContainingWindow();
	}
}

void SReplaceStringDialog::OnReplaceTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter)
	{
		ReplaceAndClose();
	}
}

FReply SReplaceStringDialog::OnReplaceClicked()
{
	ReplaceAndClose();

	return FReply::Handled();
}

FReply SReplaceStringDialog::OnCancelClicked()
{
	CloseContainingWindow();

	return FReply::Handled();
}

void SReplaceStringDialog::CloseContainingWindow()
{
	TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());

	if (CurrentWindow.IsValid())
	{
		CurrentWindow->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE
