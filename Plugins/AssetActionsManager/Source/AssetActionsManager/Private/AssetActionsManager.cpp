// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetActionsManager.h"
#include "DebugHelper.h"
#include "ContentBrowserModule.h" 
#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FAssetActionsManagerModule"

void FAssetActionsManagerModule::StartupModule()
{
	InitCBMenuExtension();
	RegisterAdvancedDeletionTab();
}

#pragma region ExtendContentBrowserMenu

void FAssetActionsManagerModule::InitCBMenuExtension()
/*
	Initialize Content Browser module and bind extender to add a custom delegate to the existing 
	array of menu options
*/
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
/*
	Instance a new FExtender that adds a menu entry by binding to a member function that creates the new entry
*/
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"), // Existing menu option 
			EExtensionHook::After, // Position of new entry relative to existing menu option
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FAssetActionsManagerModule::AddCBMenuEntry)); // Second delegate bind to member fn

		// Selected Paths is passed in through bind fn which is based on user selection of folder
		// Set an array to the path that is obtained from this menu action
		SelectedFolderPaths = SelectedPaths;
	}

	return MenuExtender;
}

void FAssetActionsManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
/*
	Utilize MenuBuilder to create a new menu entry and define menu entry properties.
	Binds to member fn that defines onClick behavior.
*/
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString("Advanced Deletion"), // Name
		FText::FromString("Search and delete assets under a selected folder"), // ToolTip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FAssetActionsManagerModule::OnAdvancedDeleteMenuEntryClicked) // Third delegate bind to member fn
	);
}

void FAssetActionsManagerModule::OnAdvancedDeleteMenuEntryClicked()
{
	// Fix up redirectors if needed
	FixUpRedirectors();

	// Spawn tab 
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}

void FAssetActionsManagerModule::FixUpRedirectors()
{
	// Load module
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// Create filter with asset paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// Query for assets in selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	// Get paths for each asset
	TArray<FString> ObjectPaths;

	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	// Load assets
	TArray<UObject*> Objects;
	bool Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, true, true);

	if (Result)
	{
		// Convert objects to object redirectors
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Call fix up redirectors from asset tools
		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}

#pragma endregion

#pragma region AdvancedDeletionTab

void FAssetActionsManagerModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"),
		FOnSpawnTab::CreateRaw(this, &FAssetActionsManagerModule::OnSpawnAdvancedDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advanced Deletion")))
		.SetAutoGenerateMenuEntry(false);
}

TSharedRef<SDockTab> FAssetActionsManagerModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& AdvancedDeletionTabArgs)
/*
	Construct an SDockTab and assign the SLATE_ARGUMENT in the widget to the asset data found in the selected folder
*/
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvancedDeletionTab)
				.AllAssetsDataFromManager(GetAllAssetDataUnderSelectedFolder()) // matches the SLATE_ARGUMENT in widget file
				.SelectedFoldersPaths(SelectedFolderPaths)
		];
}

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::GetAllAssetDataUnderSelectedFolder()
/*
	From selected folder path, list all assets and add to array. For every asset path in the array,
	find the asset data and return an array of asset data.
*/
{
	TArray<TSharedPtr<FAssetData>> AllAssetsData; // type matches what item source in widget expects
	TArray<FString> AllAssetsPathsNames;

	// Use List Assets to get all assets paths
	for (const FString SelectedFolderPath : SelectedFolderPaths)
	{
		TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(SelectedFolderPath);
		AllAssetsPathsNames.Append(AssetsPathNames);
	}

	// Check if selected folder contains assets - might move this to an if check in the widget and display no results
	if (AllAssetsPathsNames.Num() == 0)
	{
		DebugHelper::MessageDialogBox(EAppMsgType::Ok, TEXT("No assets found under selected folder"));
	}

	for (const FString AssetPathName : AllAssetsPathsNames)
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


		// Get asset data and add to array to pass to widget
		FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);

		// Skip level maps
		if (AssetData.GetClass()->GetName() == TEXT("World")) continue;

		AllAssetsData.Add(MakeShared<FAssetData>(AssetData));
	}

	return AllAssetsData;
}

#pragma endregion

#pragma region ProcessDataForWidget

bool FAssetActionsManagerModule::DeleteAssetsInList(const TArray<FAssetData>& AssetsToDelete)
/*
	Return true if assets were deleted successfully by DeleteAssets fn from ObjectTools
	Else return false
*/
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}

	return false;
}

int32 FAssetActionsManagerModule::GetAssetReferencersCount(const TSharedPtr<FAssetData>& AssetData)
{
	TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData->ObjectPath.ToString());

	return AssetReferencers.Num();
}

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::FilterForUnusedAssetData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter)
/*
	List unsused assets by checking count of asset referencers for all assets under selected folder 
*/
{
	TArray<TSharedPtr<FAssetData>> UnusedAssetsData;

	for (const TSharedPtr<FAssetData>& AssetData : AssetDataToFilter)
	{
		if (GetAssetReferencersCount(AssetData) == 0)
		{
			UnusedAssetsData.Add(AssetData);
		}
	}

	return UnusedAssetsData;
}

#pragma endregion

void FAssetActionsManagerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvancedDeletion"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetActionsManagerModule, AssetActionsManager)