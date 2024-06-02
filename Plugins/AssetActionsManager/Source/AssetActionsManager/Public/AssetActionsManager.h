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

#pragma region ProcessDataForAdvancedDeletion

	bool DeleteAssetsInList(const TArray<FAssetData>& AssetsToDelete);

#pragma endregion


private:
#pragma region ExtendContentBrowserMenu
	
	TArray<FString> SelectedFolderPaths;

	void InitCBMenuExtension();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	void OnAdvancedDeleteButtonClicked();

#pragma endregion

#pragma region AdvancedDeletionTab

	void RegisterAdvancedDeletionTab();
	TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& AdvancedDeletionTabArgs);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

#pragma endregion

};
