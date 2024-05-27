// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetActionsManager.h"
#include "DebugHelper.h"
#include "ContentBrowserModule.h" 
#include "SlateWidgets/AdvancedDeletionWidget.h"

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
		];
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetActionsManagerModule, AssetActionsManager)