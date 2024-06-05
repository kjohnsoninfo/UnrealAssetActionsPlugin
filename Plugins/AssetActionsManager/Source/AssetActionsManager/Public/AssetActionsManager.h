// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetActionsManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ProcessDataForWidget

	/** Delete assets marked for deletion by the user widget 
	* 
	* @return bool: returns true when function successfully deletes an asset, else returns false
	* 
	* @note: This is important because the widget calls different functions 
	*		 based on whether or not assets were deleted.
	*/
	bool DeleteAssetsInList(const TArray<FAssetData>& AssetsToDelete);

	/** Get unused assets by filtering all assets
	*
	* @param AssetDataToFilter: array of assets to filter on
	*/
	TArray<TSharedPtr<FAssetData>> FilterForUnusedAssetData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter);

	/** Get count of all asset referencers
	*
	* @param AssetData: asset to get referencer count for
	*/
	int32 GetAssetReferencersCount(const TSharedPtr<FAssetData>& AssetData);

#pragma endregion


private:
#pragma region ExtendContentBrowserMenu
	
	/** Array to hold folder paths of user selected folder
	* @note: This is based on the folder that the user right-clicks in the content browser
	*/
	TArray<FString> SelectedFolderPaths;

	/** Extend menu options in the right-click menu of the Content Browser */
	void InitCBMenuExtension();

	/** Delegate function to add a custom entry to the right-click menu  
	* @note: This is bound in the InitCBMenuExtension fn
	*/
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/** Delegate function to create the new menu entry added to the rigjt-click menu
	* @note: This is bound in the CustomCBMenuExtender fn
	*/
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	/** Spawn tab when the Advanced Deletion menu entry is clicked */
	void OnAdvancedDeleteMenuEntryClicked();

	/** Fix up redirectors when tab is spawned to ensure asset file paths are accurate */
	void FixUpRedirectors();
#pragma endregion

#pragma region AdvancedDeletionTab

	/** Register newly created tab  */
	void RegisterAdvancedDeletionTab();

	/** Create a new nomad tab */
	TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& AdvancedDeletionTabArgs);
	
	/** Return an array of Asset Data under the folder that the user opened the right-click menu on */
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

#pragma endregion

};
