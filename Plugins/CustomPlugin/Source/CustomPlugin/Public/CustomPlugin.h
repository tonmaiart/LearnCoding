// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

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
	/* Variables*/
	TArray<FString> FolderPathsSelected;

	/* Add Menu Entry Function */
	void InitCBMenuExtention();
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	/* On Click Menu Button Function */
	void OnDeleteUnusedAssetButtonClicked();
	void OnAlembicImporter();
	void OnShotReader();
	
	/* Extra Tools Function*/
	void OnFixUpRedirectors();
	TSharedRef<FExtender> OnRightClick(const TArray<FString>& SelectedPaths);

#pragma endregion

#pragma region CustomEditor
	void RegisterAlembicImporter();
	TSharedRef<SDockTab> OnSpawnAlembicImporterTab(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
	TArray<FString> GetDirectoryContent(FString DirectoryPath);

#pragma endregion

#pragma region ShotReader
	void RegisterShotReader();
	TSharedRef<SDockTab> OnSpawnShotReader(const FSpawnTabArgs& SpawnTabArgs);

#pragma endregion

};
