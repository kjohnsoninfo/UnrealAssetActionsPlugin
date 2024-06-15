// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetActionsManager.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetViewUtils.h"
#include "ContentBrowserModule.h" 
#include "DebugHelper.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "SlateWidgets/AssetActionsWidget.h"

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
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit"),
		FExecuteAction::CreateRaw(this, &FAssetActionsManagerModule::OnAssetActionsMenuEntryClicked) // Third delegate bind to member fn
	);
}

void FAssetActionsManagerModule::OnAssetActionsMenuEntryClicked()
/*
	Spawn the AssetActions tab when the menu entry is clicked
*/
{
	// Fix up redirectors if needed
	FixUpRedirectors();

	// Spawn tab 
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AssetActions"));
}

#pragma endregion

#pragma region AssetActionsTab

void FAssetActionsManagerModule::RegisterAssetActionsTab()
/*
	Register the spawned tab, bind the spawn behavior, and set the tab properties
*/
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AssetActions"),
		FOnSpawnTab::CreateRaw(this, &FAssetActionsManagerModule::OnSpawnAssetActionsTab))
		.SetDisplayName(FText::FromString(TEXT("Quick Asset Actions")))
		.SetAutoGenerateMenuEntry(false)
		.SetReuseTabMethod(FOnFindTabToReuse::CreateLambda([&](const FTabId&) // set behavior if tab already exists
			{
				// tab exists, close it and set it to nullptr so that it reopens
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
/*
	Return a count of all the asset references found for AssetData
*/
{
	TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData->GetSoftObjectPath().GetLongPackageName());

	return AssetReferencers.Num();
}

void FAssetActionsManagerModule::SyncCBToClickedAsset(const FString& ClickedAssetPath)
/*
	Navigate the content browser to the ClickedAssetPath
*/
{
	// Convert path to array since that is what SyncBrowserToObjects expects
	TArray<FString> AssetPathToSync;
	AssetPathToSync.Add(ClickedAssetPath);

	UEditorAssetLibrary::SyncBrowserToObjects(AssetPathToSync);
}

void FAssetActionsManagerModule::FixUpRedirectors()
/*
	Fix up any redirectors created on the whole project
*/
{
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

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::FilterForUnusedAssetData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter)
/*
	Return an array of unsused assets by checking count of asset referencers for all assets under selected folder 
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
/*
	Return an array of duplicate name assets by using a multimap to find asset names with more than one value
*/
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

TArray<TSharedPtr<FAssetData>> FAssetActionsManagerModule::FilterForNoPrefixData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter)
/*
	Return an array of assets with no prefix (or incorrect prefix) by checking asset names against the prefix map
*/
{
	TArray<TSharedPtr<FAssetData>> NoPrefixAssets;

	for (const TSharedPtr<FAssetData>& Asset : AssetDataToFilter)
	{
		FString* PrefixFound = PrefixesMap.Find(Asset->GetClass()->GetName());
		FString AssetName = Asset->AssetName.ToString();

		// skip maps
		if (Asset->GetClass()->GetName() == TEXT("World")) { continue; }

		if (!PrefixFound)
		{
			DebugHelper::PrintLog("No prefix found for asset: " + AssetName);
			continue;
		}

		else
		{
			if (!AssetName.Contains(*PrefixFound))
			{
				NoPrefixAssets.Add(Asset);
			}
		}
	}

	return NoPrefixAssets;
}

bool FAssetActionsManagerModule::DeleteAssetsInList(const TArray<FAssetData>& AssetsToDelete)
/*
	Return true if assets were successfully deleted; else return false
*/
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}

	return false;
}

bool FAssetActionsManagerModule::RenameAssetInList(const FString& NewName, const TSharedPtr<FAssetData>& AssetToRename)
/*
	Return true if assets were successfully renamed; else return false
*/
{
	const FString OldAssetPath = AssetToRename->GetObjectPathString();
	const FString NewAssetPath = FPaths::Combine(AssetToRename->PackagePath.ToString(), NewName);

	if (UEditorAssetLibrary::RenameAsset(OldAssetPath, NewAssetPath))
	{
		UEditorAssetLibrary::SaveAsset(NewAssetPath, false);
		return true;
	}

	return false;
}

bool FAssetActionsManagerModule::DuplicateAssetsInList(int32 NumOfDuplicates, const TArray<TSharedPtr<FAssetData>>& AssetsToDuplicate)
/*
	Return true if assets were successfully duplicated; else return false
*/
{
	uint32 Count = 0;

	if (NumOfDuplicates <= 0) { return false; }

	for (const TSharedPtr<FAssetData>& AssetToDuplicate : AssetsToDuplicate)
	{
		for (int32 i = 0; i < NumOfDuplicates; ++i)
		{
			FString SourceAssetPath = AssetToDuplicate->GetObjectPathString();
			// Avoid TryConvertFilenameToLongPackageName warning
			TArray<FString> PathNameArray;
			SourceAssetPath.ParseIntoArray(PathNameArray, TEXT("."));
			SourceAssetPath = PathNameArray[0];

			// Get existing assets to check name
			TArray<TSharedPtr<FAssetData>> AllAssets = GetAllAssetDataUnderSelectedFolder();
			TArray<FString> AssetNames;

			for (TSharedPtr<FAssetData> Asset : AllAssets)
			{
				AssetNames.AddUnique(Asset->AssetName.ToString());
			}

			FString DuplicatedAssetName = AssetToDuplicate->AssetName.ToString() + FString::FromInt(i + 1);

			// if duplicated name already found, add number to suffix
			for (int32 j = 1; j < AssetNames.Num(); ++j)
			{
				if (AssetNames[j] == DuplicatedAssetName)
				{
					DuplicatedAssetName = AssetToDuplicate->AssetName.ToString() + FString::FromInt(i + 1 + j);
				}
			}

			const FString NewAssetPath = FPaths::Combine(AssetToDuplicate->PackagePath.ToString(), DuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewAssetPath))
			{
				UEditorAssetLibrary::SaveAsset(NewAssetPath, false);
				++Count;
			}
		}
	}

	if (Count > 0)
	{
		return true;
	}
	
	return false;
}

bool FAssetActionsManagerModule::AddPrefixesToAssetsInList(const TArray<TSharedPtr<FAssetData>>& AssetsToAddPrefixes)
/*
	Return true if prefixes were successfully added to asset names; else return false
*/
{
	uint32 Count = 0;
	for (const TSharedPtr<FAssetData>& Asset : AssetsToAddPrefixes)
	{
		FString* PrefixFound = PrefixesMap.Find(Asset->GetClass()->GetName());
		FString OldName = Asset->AssetName.ToString();

		// skip maps
		if (Asset->GetClass()->GetName() == TEXT("World")) { continue; }

		if (!PrefixFound)
		{
			DebugHelper::PrintLog("No prefix found for asset: " + OldName);
			continue;
		}

		if (OldName.StartsWith(*PrefixFound))
		{
			DebugHelper::PrintLog(OldName + " already has prefix added");
			continue;
		}

		// Remove Unreal default _Inst for Material Instances
		if (Asset->GetClass()->GetName() == TEXT("MaterialInstanceConstant"))
		{
			if (OldName.Contains(TEXT("_Inst")))
			{

				TArray<FString> NameArray;
				OldName.ParseIntoArray(NameArray, TEXT("_Inst"));

				OldName.Empty();

				for (FString Name : NameArray)
				{
					OldName.Append(Name);
				}
			}

			OldName.RemoveFromStart(TEXT("M_"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		FString OldAssetPath = Asset->GetObjectPathString();
		
		// Avoid TryConvertFilenameToLongPackageName warning
		TArray<FString> PathNameArray;
		OldAssetPath.ParseIntoArray(PathNameArray, TEXT("."));
		OldAssetPath = PathNameArray[0];

		const FString NewAssetPath = FPaths::Combine(Asset->PackagePath.ToString(), NewNameWithPrefix);

		if (UEditorAssetLibrary::RenameAsset(OldAssetPath, NewAssetPath))
		{
			UEditorAssetLibrary::SaveAsset(NewAssetPath, false);
			++Count;
		}

	}

	if (Count > 0)
	{
		return true;
	}
	
	return false;
}

bool FAssetActionsManagerModule::ReplaceString(const FString& OldString, const FString& NewString, const TArray<TSharedPtr<FAssetData>>& AssetsToReplace)
/*
	Return true if user entered string to replace was successfully replaced with new string; else return false
*/
{
	uint32 Count = 0;

	for (const TSharedPtr<FAssetData>& Asset : AssetsToReplace)
	{
		const FString OldAssetName = Asset->AssetName.ToString();

		if (OldAssetName.Contains(OldString, ESearchCase::CaseSensitive))
		{
			FString NewAssetName = OldAssetName.Replace(*OldString, *NewString, ESearchCase::CaseSensitive);

			FString OldAssetPath = Asset->GetObjectPathString();
			
			// Avoid TryConvertFilenameToLongPackageName warning
			TArray<FString> PathNameArray;
			OldAssetPath.ParseIntoArray(PathNameArray, TEXT("."));
			OldAssetPath = PathNameArray[0];

			const FString NewAssetPath = FPaths::Combine(Asset->PackagePath.ToString(), NewAssetName);

			if (UEditorAssetLibrary::RenameAsset(OldAssetPath, NewAssetPath))
			{
				UEditorAssetLibrary::SaveAsset(NewAssetPath, false);
				++Count;
			}
		}
	}

	if (Count > 0)
	{
		return true;
	}
	
	return false;
}

#pragma endregion

void FAssetActionsManagerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AssetActions"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetActionsManagerModule, AssetActionsManager)