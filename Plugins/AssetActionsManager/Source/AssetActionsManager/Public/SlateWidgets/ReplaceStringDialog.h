// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
namespace ETextCommit { enum Type : int; }

class SReplaceStringDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SReplaceStringDialog)
		: _Padding(FMargin(15))
		{}
		SLATE_ARGUMENT(FMargin, Padding);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FString OldString;
	FString NewString;

private:
	/** Renames the asset based on dialog parameters */
	FReply OnReplaceClicked();

	/** Sends string input data and attempts to close the active window */
	void ReplaceAndClose();

	/** Callback for when Cancel is clicked */
	FReply OnCancelClicked();

	/** Attempts to replace if enter is pressed while editing the input strings */
	void OnReplaceTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/** Closes the window that contains this widget */
	void CloseContainingWindow();

private:

	TSharedPtr<SEditableTextBox> OldStringTextBox;

	TSharedPtr<SEditableTextBox> NewStringTextBox;

};
