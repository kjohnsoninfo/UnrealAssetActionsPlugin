// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Modules/ModuleManager.h"

class FAssetActionsManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ProcessDataForWidget

	/** Get an array of Asset Data under the folder that the user opened the right-click menu on */
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
	
	/** Get count of all asset referencers for single asset */
	int32 GetAssetReferencersCount(const TSharedPtr<FAssetData>& AssetData);

	/** Get unused assets by filtering all assets */
	TArray<TSharedPtr<FAssetData>> FilterForUnusedAssetData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter);

	/** Get assets with duplicate names by filtering all assets */
	TArray<TSharedPtr<FAssetData>> FilterForDuplicateNameData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter);

	/** Get assets with no prefix by filtering all assets */
	TArray<TSharedPtr<FAssetData>> FilterForNoPrefixData(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter);

	/** Delete assets marked for deletion by the user widget
	 *
	 * @return bool: returns true when function successfully deletes an asset, else returns false
	 *
	 * @note: This is important because the widget calls different functions
	 *		 based on whether or not assets were deleted.
	 */
	bool DeleteAssetsInList(const TArray<FAssetData>& AssetsToDelete);

	/** Move content browser location to asset path passed in by widget */
	void SyncCBToClickedAsset(const FString& ClickedAssetPath);

	/** Rename assets selected in the user widget */
	bool RenameAssetInList(const FString& NewName, const TSharedPtr<FAssetData>& AssetToRename);

	/** Duplicate assets selected in the user widget */
	bool DuplicateAssetsInList(int32 NumOfDuplicates, const TArray<TSharedPtr<FAssetData>>& AssetsToDuplicate);

	/** Add prefixes to assets selected in the user widget */
	bool AddPrefixesToAssetsInList(const TArray<TSharedPtr<FAssetData>>& AssetsToAddPrefixes);

	/** Add prefixes to assets selected in the user widget */
	bool ReplaceString(const FString& OldString, const FString& NewString, const TArray<TSharedPtr<FAssetData>>& AssetsToReplace);

	/** Fix up redirectors to ensure asset file paths are accurate */
	void FixUpRedirectors();
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
	void OnAssetActionsMenuEntryClicked();

#pragma endregion

#pragma region AssetActionsTab

	/** Pointer to created tab  */
	TSharedPtr<SDockTab> AssetActionsTab;

	/** Register newly created tab  */
	void RegisterAssetActionsTab();

	/** Create a new nomad tab */
	TSharedRef<SDockTab> OnSpawnAssetActionsTab(const FSpawnTabArgs& AssetActionsTabArgs);

#pragma endregion

	TMap<FString, FString> PrefixesMap =
	{
		{ TEXT("AimOffsetBlendSpace"), TEXT("AO_")},
		{ TEXT("AnimBlueprint"), TEXT("ABP_") },
		{ TEXT("AnimMontage"), TEXT("AM_") },
		{ TEXT("AnimSequence"), TEXT("AS_") },
		{ TEXT("BehaviorTree"), TEXT("BT_")},
		{ TEXT("BlackboardData"), TEXT("BB_") },
		{ TEXT("BlendSpace"), TEXT("BS_") },
		{ TEXT("BlendSpace1D"), TEXT("BS_") },
		{ TEXT("Blueprint"), TEXT("BP_")},
		{ TEXT("ControlRigBlueprint"),TEXT("CRBP_") },
		{ TEXT("CurveFloat"), TEXT("Curve_") },
		{ TEXT("CurveLinearColor"), TEXT("Curve_") },
		{ TEXT("CurveLinearColorAtlas"), TEXT("Curve_") },
		{ TEXT("CurveVector"), TEXT("Curve_") },
		{ TEXT("DataTable"),TEXT("DT_") },
		{ TEXT("FoliageType_InstancedStaticMesh"), TEXT("FT_") },
		{ TEXT("Font"), TEXT("Font_")},
		{ TEXT("FontFace"), TEXT("Font_")},
		{ TEXT("IKRetargeter"), TEXT("IK_") },
		{ TEXT("IKRigDefinition"), TEXT("IK_") },
		{ TEXT("LandscapeGrassType"), TEXT("LG_") },
		{ TEXT("LandscapeLayerInfoObject"), TEXT("LL_") },
		{ TEXT("LevelSequence"), TEXT("LS_") },
		{ TEXT("MapBuildDataRegistry"), TEXT("MapBD_") },
		{ TEXT("Material"), TEXT("M_") },
		{ TEXT("MaterialFunction"), TEXT("MF_") },
		{ TEXT("MaterialInstanceConstant"), TEXT("MI_") },
		{ TEXT("MaterialParameterCollection"), TEXT("MPC_") },
		{ TEXT("ParticleSystem"), TEXT("PS_") },
		{ TEXT("PhysicalMaterial"), TEXT("PM_") },
		{ TEXT("PhysicsAsset"), TEXT("PHYS_") },
		{ TEXT("PoseAsset"), TEXT("POSE_") },
		{ TEXT("ProceduralFoliageSpawner"), TEXT("FS_") },
		{ TEXT("SkeletalMesh"), TEXT("SM_") },
		{ TEXT("SkeletalMeshLODSettings"), TEXT("SK_") },
		{ TEXT("SkeletalMeshLODSettings"), TEXT("SK_") },
		{ TEXT("Skeleton"), TEXT("SKEL_") },
		{ TEXT("SoundClass"), TEXT("SC_") },
		{ TEXT("SoundCue"), TEXT("SFX_") },
		{ TEXT("SoundMix"), TEXT("SMix_") },
		{ TEXT("SoundWave"), TEXT("SFX_") },
		{ TEXT("StaticMesh"), TEXT("SM_") },
		{ TEXT("SubsurfaceProfile"), TEXT("SSP_") },
		{ TEXT("Texture"), TEXT("T_")},
		{ TEXT("Texture2D"), TEXT("T_")},
		{ TEXT("TextureCube"), TEXT("TC_") },
		{ TEXT("TextureRenderTarget"), TEXT("RT_") },
		{ TEXT("TextureRenderTarget2D"), TEXT("RT_") },
		{ TEXT("UserDefinedEnum"), TEXT("E_") },
		{ TEXT("UserDefinedStruct"), TEXT("S_") },
		{ TEXT("VectorFieldStatic"), TEXT("VF_") },
		{ TEXT("WidgetBlueprint"), TEXT("WBP_") },
	};

};
