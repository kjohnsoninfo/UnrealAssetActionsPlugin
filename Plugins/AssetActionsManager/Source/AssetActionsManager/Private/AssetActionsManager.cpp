// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetActionsManager.h"
#include "DebugHelper.h"
#include "ContentBrowserModule.h" 
#include "SlateWidgets/AssetActionsWidget.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FAssetActionsManagerModule"

void FAssetActionsManagerModule::StartupModule()
{
	InitCBMenuExtension();
	RegisterAssetActionsTab();
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
			FName("Delete"), // Existing menu option (extension points option in editor prefs)
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
		FText::FromString("Quick Asset Actions"), // Name
		FText::FromString("Spawn a list of assets under a selected folder to filter and perform bulk actions on"), // ToolTip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FAssetActionsManagerModule::OnAssetActionsMenuEntryClicked) // Third delegate bind to member fn
	);
}

void FAssetActionsManagerModule::OnAssetActionsMenuEntryClicked()
{
	// Fix up redirectors if needed
	FixUpRedirectors();

	// Spawn tab 
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AssetActions"));
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

#pragma region AssetActionsTab

void FAssetActionsManagerModule::RegisterAssetActionsTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AssetActions"),
		FOnSpawnTab::CreateRaw(this, &FAssetActionsManagerModule::OnSpawnAssetActionsTab))
		.SetDisplayName(FText::FromString(TEXT("Quick Asset Actions")))
		.SetAutoGenerateMenuEntry(false)
		.SetReuseTabMethod(FOnFindTabToReuse::CreateLambda([&](const FTabId&)
			{
				if (AssetActionsTab.IsValid())
				{
					AssetActionsTab->RequestCloseTab();
					AssetActionsTab = nullptr;
				}

				return AssetActionsTab;
			}));
}

TSharedRef<SDockTab> FAssetActionsManagerModule::OnSpawnAssetActionsTab(const FSpawnTabArgs& AssetActionsTabArgs)
/*
	Construct an SDockTab and assign the SLATE_ARGUMENT in the widget to the asset data found in the selected folder
*/
{
	AssetActionsTab =
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAssetActionsTab)
				.AllAssetsDataFromManager(GetAllAssetDataUnderSelectedFolder()) // matches the SLATE_ARGUMENT in widget file
				.SelectedFoldersPaths(SelectedFolderPaths)
		];

	return AssetActionsTab.ToSharedRef();
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

	for (FString AssetPathName : AllAssetsPathsNames)
	{
		// Don't delete any required UE assets
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		// Avoid TryConvertFilenameToLongPackageName warning
		TArray<FString> PathNameArray;
		AssetPathName.ParseIntoArray(PathNameArray, TEXT("."));
		AssetPathName = PathNameArray[0];

		// If asset does not exist, skip and move to next asset
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;


		// Get asset data and add to array to pass to widget
		FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AllAssetsData.Add(MakeShared<FAssetData>(AssetData));
	}

	return AllAssetsData;
}

#pragma endregion

#pragma region ProcessDataForWidget

int32 FAssetActionsManagerModule::GetAssetReferencersCount(const TSharedPtr<FAssetData>& AssetData)
{
	TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData->GetSoftObjectPath().GetLongPackageName());

	return AssetReferencers.Num();
}

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

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::FilterForUnusedAssetData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter)
/*
	List unsused assets by checking count of asset referencers for all assets under selected folder 
*/
{
	TArray<TSharedPtr<FAssetData>> UnusedAssetsData;

	for (const TSharedPtr<FAssetData>& AssetData : AssetDataToFilter)
	{

		// Skip level maps
		if (AssetData->GetClass()->GetName() == TEXT("World")) continue;

		if (GetAssetReferencersCount(AssetData) == 0)
		{
			UnusedAssetsData.Add(AssetData);
		}
	}

	return UnusedAssetsData;
}

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::FilterForDuplicateNameData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter)
{
	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMap;
	TArray<TSharedPtr<FAssetData>> DuplicatedAssetData;

	for (const TSharedPtr<FAssetData>& AssetData : AssetDataToFilter)
	{
		AssetsInfoMap.Emplace(AssetData->AssetName.ToString(), AssetData);
	}

	for (const TSharedPtr<FAssetData>& AssetData : AssetDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> FoundNamesArray;

		AssetsInfoMap.MultiFind(AssetData->AssetName.ToString(), FoundNamesArray);

		if (FoundNamesArray.Num() <= 1) continue;

		for (const TSharedPtr<FAssetData>& DuplicatedNameData : FoundNamesArray)
		{
			if (DuplicatedNameData.IsValid())
			{
				DuplicatedAssetData.AddUnique(DuplicatedNameData);
			}
		}
	}

	return DuplicatedAssetData;
}

#pragma endregion

void FAssetActionsManagerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AssetActions"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetActionsManagerModule, AssetActionsManager)