// Copyright Epic Games, Inc. All Rights Reserved.

#include "UkoreToolkit.h"
#include "UkoreAssetImporter/UkoreAssetImporter.h"
#include "UkoreAssetImporter/UkoreAssetImporterWidgetTab.h"

#define LOCTEXT_NAMESPACE "FUkoreToolkitModule"

void FUkoreToolkitModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->AddSection("UkoreToolkit", FText::FromString("Ukore Toolkit"));
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"UkoreToolkit",
		FText::FromString("Ukore Toolkit"),
		FText::FromString("Open the asset importer tool"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
		FUIAction(FExecuteAction::CreateRaw(this, &FUkoreToolkitModule::OnSpawnUkoreAssetImporterClicked))
		);
	Section.AddEntry(Entry);
}

void FUkoreToolkitModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUkoreToolkitModule::RegisterAssetImporter();
}

void FUkoreToolkitModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FUkoreToolkitModule::OnSpawnUkoreAssetImporterClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("UkoreAssetImporter"));
}

TSharedRef<SDockTab> FUkoreToolkitModule::OnSpawnUkoreAssetImporter(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUkoreAssetImporterWidgetTab)
		];
}

void FUkoreToolkitModule::RegisterAssetImporter()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("UkoreAssetImporter"),
		FOnSpawnTab::CreateRaw(this, &FUkoreToolkitModule::OnSpawnUkoreAssetImporter))
		.SetDisplayName(FText::FromString(TEXT("Ukore Asset Importer")));

}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUkoreToolkitModule, UkoreToolkit)

# pragma endregion