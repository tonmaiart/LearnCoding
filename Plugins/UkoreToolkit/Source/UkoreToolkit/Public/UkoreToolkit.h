// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUkoreToolkitModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();

	void RegisterAssetImporter();
	TSharedRef<SDockTab> OnSpawnUkoreAssetImporter(const FSpawnTabArgs& SpawnTabArgs);
	void OnSpawnUkoreAssetImporterClicked();
};
