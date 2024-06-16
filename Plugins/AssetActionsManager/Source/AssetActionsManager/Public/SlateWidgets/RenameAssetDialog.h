// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
namespace ETextCommit { enum Type : int; }

class SRenameAssetDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRenameAssetDialog)
		: _AssetData()
		, _Padding(FMargin(15))
		{}
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, AssetData)
			SLATE_ARGUMENT(FMargin, Padding)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FString NewName;

private:
	/** Renames the asset based user input */
	FReply OnRenameClicked();

	/** Returns new name and attempts to close the active window */
	FString RenameAndClose();

	/** Callback for when Cancel is clicked */
	FReply OnCancelClicked();

	/** Attempts to rename if enter is pressed while editing the asset name */
	void OnRenameTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/** Closes the window that contains this widget */
	void CloseContainingWindow();

private:

	TSharedPtr<FAssetData> AssetData;

	TSharedPtr<SEditableTextBox> NewNameTextBox;

};
