// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCustomPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ProcessActionAlembicImporter

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);


#pragma endregion
private:

#pragma region ContentBrowserMenuExtention

	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;

	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);



	void OnDeleteUnusedAssetButtonClicked();
	
	void OnFixUpRedirectors();

	void OnAlembicImporter();

	TSharedRef<FExtender> OnRightClick(const TArray<FString>& SelectedPaths);
		
#pragma endregion

#pragma region CustomEditor
	void  RegisterAlembicImporter();

	TSharedRef<SDockTab> OnSpawnAlembicImporterTab(const FSpawnTabArgs& SpawnTabArgs);
	
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

#pragma endregion


};
