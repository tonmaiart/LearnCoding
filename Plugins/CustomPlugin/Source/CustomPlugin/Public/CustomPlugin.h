// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "SlateWidgets/ShotReader.h"

class FCustomPluginModule : public IModuleInterface
{
public:
	FString ShotRootPath = "G:\\My Drive\\Projects\\KafkaProj\\publish\\Shot";
	FString SubFolder = "Layout";

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

#pragma endregion

#pragma region Utilites
	//TArray<FString> GetDirectoryContent(FString DirectoryPath, bool GetDir = true, bool GetFile = true);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
#pragma endregion

#pragma region AlembicImporter
	void RegisterAlembicImporter();
	TSharedRef<SDockTab> OnSpawnAlembicImporterTab(const FSpawnTabArgs& SpawnTabArgs);
#pragma endregion

#pragma region ShotReader
	void RegisterShotReader();
	TSharedRef<SDockTab> OnSpawnShotReader(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedRef<FShotData>> GetShotData(); // Use For Get All Need Data for update shot
	
#pragma endregion

};
