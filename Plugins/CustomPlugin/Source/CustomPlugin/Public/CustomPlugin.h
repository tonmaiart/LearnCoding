// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCustomPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

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

	TSharedRef<SDockTab> OnSpawnAlembicImporterTab(const FSpawnTabArgs& SpawnTabArg);
	
#pragma endregion

};
