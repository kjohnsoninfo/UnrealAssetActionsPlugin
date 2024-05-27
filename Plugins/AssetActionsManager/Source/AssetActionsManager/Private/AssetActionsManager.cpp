// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetActionsManager.h"
#include "DebugHelper.h"
#include "ContentBrowserModule.h" 
#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "EditorAssetLibrary.h"

#define LOCTEXT_NAMESPACE "FAssetActionsManagerModule"

void FAssetActionsManagerModule::StartupModule()
{
	InitCBMenuExtension();
	RegisterAdvancedDeletionTab();
}

void FAssetActionsManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ExtendContentBrowserMenu

void FAssetActionsManagerModule::InitCBMenuExtension()
{
	// Load CB module
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Get array of all menu options
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// Add custom option to array
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FAssetActionsManagerModule::CustomCBMenuExtender); // First delegate bind to member fn
	ContentBrowserMenuExtenders.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FAssetActionsManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FAssetActionsManagerModule::AddCBMenuEntry)); // Second delegate bind to member fn

		// Selected Paths is passed in through bind fn which is based on user selection of folder
		// Create an array and set it to the path that is obtained from this menu action
		SelectedFolderPaths = SelectedPaths;
	}

	return MenuExtender;
}

void FAssetActionsManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString("Advanced Deletion"), // Name
		FText::FromString("Search and delete assets under a selected folder"), // ToolTip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FAssetActionsManagerModule::OnAdvancedDeleteButtonClicked) // Third delegate bind to member fn
	);
}

void FAssetActionsManagerModule::OnAdvancedDeleteButtonClicked()
{
	// Spawn tab 
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}

#pragma endregion

#pragma region AdvancedDeletionTab

void FAssetActionsManagerModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"),
		FOnSpawnTab::CreateRaw(this, &FAssetActionsManagerModule::OnSpawnAdvancedDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advanced Deletion")));
}

TSharedRef<SDockTab> FAssetActionsManagerModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& AdvancedDeletionTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvancedDeletionTab)
				.AssetsDataFromManager(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AllAssetsData; // type matches what item source in widget expects

	// Restrict folder selection to one to mitigate errors
	if (SelectedFolderPaths.Num() > 1)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("Please select only one folder."));
		return AllAssetsData;
	}

	// Use List Assets to get all assets paths
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0]);

	// Check if selected folder contains assets - might move this to an if check in the widget and display no results
	if (AssetsPathNames.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets found under selected folder"));
	}

	for (const FString AssetPathName : AssetsPathNames)
	{
		// Don't delete any required UE assets
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		// If asset does not exist, skip and move to next asset
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		// Get asset data
		FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AllAssetsData.Add(MakeShared<FAssetData>(AssetData));
	}

	return AllAssetsData;
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetActionsManagerModule, AssetActionsManager)