// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomPlugin.h"
#include "DebugHeader.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "SlateWidgets/AlembicImporterWidget.h"
#include "SlateWidgets/ShotReader.h"
#include "Misc/Paths.h"
#include "SimpleUtilities.h"

//#include <iostream>
//#include <filesystem>
#include "GenericPlatform/GenericPlatformFile.h"

#define LOCTEXT_NAMESPACE "FCustomPluginModule"

void FCustomPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	InitCBMenuExtention();
	RegisterAlembicImporter();
	RegisterShotReader();
}

#pragma region ContentBrowserMenuExtention
void FCustomPluginModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule= 
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FCustomPluginModule::CustomCBMenuExtender));


}

TSharedRef<FExtender> FCustomPluginModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	// Check for selection
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FCustomPluginModule::AddCBMenuEntry));

		FolderPathsSelected = SelectedPaths;

	}
	return MenuExtender;
}



void FCustomPluginModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Asset")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FCustomPluginModule::OnDeleteUnusedAssetButtonClicked)
	);


	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Alembic Importer")),
		FText::FromString(TEXT("Used for Check Alembic Import List")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FCustomPluginModule::OnAlembicImporter)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Shot Reader")),
		FText::FromString(TEXT("Check new version of animation / camera file in content directory to update.")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FCustomPluginModule::OnShotReader)
	);


}

void FCustomPluginModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathsSelected.Num()!=1) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this action to single selected folder."));
		return;
	}

	DebugHeader::Print(TEXT("Currently Selected Fodler : ")+FolderPathsSelected[0],FColor::Green);

	TArray<FString> AssetsPathNames = 
	UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, false);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No Assets found under selected folder."));
		return;
	}

	EAppReturnType::Type ConfirmResult = 
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found.\n Would You like to proceed?"));
	
	if (ConfirmResult == EAppReturnType::No) return;

	OnFixUpRedirectors();
	TArray<FAssetData> UnusedAssetsData;

	for (const FString& AssetPath : AssetsPathNames)
	{

		// Don't touch root folder
		if (AssetPath.Contains(TEXT("Developers")) || AssetPath.Contains(TEXT("Collections")))
		{
			continue;
		}

		// Recheck is folder really exists
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath)) continue;


		// Add Folder Asset data to unusedassets data if have no used data
		TArray<FString> ReferencersPathes =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath, true);


		if (ReferencersPathes.Num() == 0)
		{
			UnusedAssetsData.Add(UEditorAssetLibrary::FindAssetData(AssetPath));
		}

	}

	// Delete Unused Assets Data
	if (!UnusedAssetsData.IsEmpty())
	{
		ObjectTools::DeleteAssets(UnusedAssetsData);
	}
}

void FCustomPluginModule::OnFixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackageNames.Emplace("/Game");
	Filter.ClassPaths.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule =
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);

}

void FCustomPluginModule::OnAlembicImporter()
{
	DebugHeader::Print(TEXT("Alembic Importer"), FColor::White);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AlembicImporter"));
}

void FCustomPluginModule::OnShotReader()
{
	DebugHeader::Print(TEXT("Launching Shot Reader UI."), FColor::White);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("ShotReader"));
}


#pragma endregion

#pragma region CustomEditor
void FCustomPluginModule::RegisterAlembicImporter()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AlembicImporter"),
		FOnSpawnTab::CreateRaw(this, &FCustomPluginModule::OnSpawnAlembicImporterTab))
					.SetDisplayName(FText::FromString(TEXT("AlembicImporter"))
					);
}

TSharedRef<SDockTab> FCustomPluginModule::OnSpawnAlembicImporterTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::PanelTab)
		[
			SNew(SAlembicImporterWidgetTab)
				.AssetDataToStore(GetAllAssetDataUnderSelectedFolder())
		]
		;
}

TArray<TSharedPtr<FAssetData>> FCustomPluginModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	
	// Iterate Each Asset Paths
	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Check Valid Folder
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetData.Add(MakeShared<FAssetData>(Data));

	}


	return AvaiableAssetData;
}

#pragma endregion

#pragma region ProcessActionAlembicImporter

bool FCustomPluginModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataArray, true)>0)
	{
		return true;
	}
	else
	{
		return false;
	}

}
#pragma endregion

#pragma region ShotReader
void FCustomPluginModule::RegisterShotReader()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("ShotReader"),
		FOnSpawnTab::CreateRaw(this, &FCustomPluginModule::OnSpawnShotReader))
		.SetDisplayName(FText::FromString(TEXT("ShotReader")));
	
}

TArray<TSharedPtr<FShotData>> FCustomPluginModule::GetShotData()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> ShotListPathList;
	TArray<FString>DirectoryContent = Utility::GetDirectoryContent(ShotRootPath, true, false);

	// loop each shot main name to get shot main path
	for (const FString& ShotMainDirPath : DirectoryContent)
	{
		// loop each shot name
		DebugHeader::Print("# Directory To Search : " + ShotMainDirPath);
		TArray<FString> ShotSubDirList = Utility::GetDirectoryContent(ShotMainDirPath,true,false);
		ShotListPathList.Append(ShotSubDirList);

		// Add Shot List Path
		for (const FString ShotSubPath : ShotSubDirList)
		{
			DebugHeader::Print("- Found Sub Dir - " + ShotSubPath);
		}
	}

	// loop for each shot list name to get shot list path
	TArray<TSharedPtr<FShotData>> ShotDataListResult;

	for (const FString& ShotListPath : ShotListPathList)
	{
		// Get Lastest Version Folder
		FString ShotListExtraPath = FPaths::Combine(ShotListPath,SubFolder);
		Utility::FVersionResult LastestVersionData = Utility::GetLastestVersionFolder(ShotListExtraPath);
		FString LastestVersionPath = LastestVersionData.Path;
		int32 LastestVersion = LastestVersionData.Version;
		
		DebugHeader::Print("- Shot List Version Path : " + ShotListPath);
		DebugHeader::Print("- Get Lastest Version Path : " + LastestVersionPath);

		if (LastestVersionPath.IsEmpty())
		{
			continue; // Skip this folder because have no version folder detected
		}

		// Get File of given lastest path
		TArray<FString> FileList = Utility::GetDirectoryContent(LastestVersionPath,false,true);

		for (const FString& ShotFile : FileList)
		{

			//Create FShotData
			TSharedPtr<FShotData> CurrentShotData = MakeShared<FShotData>();
			CurrentShotData->ShotMainName = FPaths::GetCleanFilename(ShotListPath);;
			CurrentShotData->ShotName = FPaths::GetBaseFilename(ShotListPath);
			CurrentShotData->AssetName = FPaths::GetBaseFilename(ShotFile);
			CurrentShotData->LastestFilePath = ShotFile;
			
			DebugHeader::Print("- Detect File Anim - " + ShotFile);

			ShotDataListResult.Add(CurrentShotData);
		}

		// 

	}
	return ShotDataListResult;
}

TSharedRef<SDockTab> FCustomPluginModule::OnSpawnShotReader(const FSpawnTabArgs& SpawnTabArgs)
{
	

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SShotReaderWidgetTab)
				.ShotDataList(FCustomPluginModule::GetShotData())
		];
}

#pragma endregion
void FCustomPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomPluginModule, CustomPlugin)