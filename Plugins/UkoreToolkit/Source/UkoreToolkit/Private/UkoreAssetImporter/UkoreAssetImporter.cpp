// Fill out your copyright notice in the Description page of Project Settings.

#include "UkoreAssetImporter/UkoreAssetImporter.h"
#include "UkoreAssetImporter/UkoreAssetImporterWidgetTab.h"

#include "UkoreToolkit.h"
#include "UkoreCore/UkoreUtility.h"

//#include "SlateWidgets/ShotReader.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"

#include "Modules/ModuleManager.h"
#include "IAssetTools.h"

#include "GeometryCache.h"
#include "AlembicImportFactory.h"
#include "AbcImportSettings.h"
#include "CoreMinimal.h"

#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "MovieScene.h"
#include "MovieSceneToolHelpers.h"
#include "MovieScenePossessable.h"
#include "MovieSceneToolsUserSettings.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "EditorReimportHandler.h"

namespace AssetImporterUtility{
TArray<TSharedPtr<FShotData>> GetShotDataArray(
	TArray<FString> ShotRootPaths,
	FString SubFolder,
	TMap<FString, FString> namingTypes,
	FString ContentShotRootPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<TSharedPtr<FShotData>> ShotDataListResult; // Used for return array of shot data

	// Debug
	if (ShotRootPaths.IsEmpty()) {
		UE_LOG(LogTemp, Log, TEXT("Shoot Root Paths is empty, Please Check the configuration."));
	}

	// Start Iterate all path in shotrootpaths
	for (FString ShotRootPath : ShotRootPaths)
	{
		FPaths::NormalizeDirectoryName(ShotRootPath);

		UE_LOG(LogTemp, Log, TEXT("Searching Shot in %s"), *ShotRootPath);

		TArray<FString> DirectoryContent = Utility::GetDirectoryContent(ShotRootPath, true, false);

		// loop each shot main name to get shot main path
		TArray<FString> ShotListPathList;
		for (const FString& ShotMainDirPath : DirectoryContent)
		{
			// loop each shot name
			DebugHeader::Print("# Directory To Search : " + ShotMainDirPath);
			TArray<FString> ShotSubDirList = Utility::GetDirectoryContent(ShotMainDirPath, true, false);
			ShotListPathList.Append(ShotSubDirList);

			// Add Shot List Path
			for (const FString ShotSubPath : ShotSubDirList)
			{
				DebugHeader::Print("- Found Sub Dir - " + ShotSubPath);
			}
		}

		// loop for each shot list name to get shot list path
		for (const FString& ShotListPath : ShotListPathList)
		{
			// Create Fasset Registry Module

			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

			// Get Lastest Version Folder
			FString ShotListExtraPath = FPaths::Combine(ShotListPath, SubFolder);
			Utility::FVersionResult LastestVersionData = Utility::GetLastestVersionFolder(ShotListExtraPath);
			FString LastestVersionPath = LastestVersionData.Path;

			if (LastestVersionPath.IsEmpty())
			{
				UE_LOG(LogTemp, Log, TEXT("Skipped Shot :%s because not found any version folder."), *LastestVersionPath);
				continue; // Skip this folder because have no version folder detected
			}

			// Get File of given lastest path
			TArray<FString> FileList = Utility::GetDirectoryContent(LastestVersionPath, false, true);

			for (const FString& ShotFile : FileList)
			{

				bool bIsValidNaming = false;

				// Check is Valid Naming Convention
				for (const TPair<FString, FString> naming : namingTypes)
				{
					FString ShotFileCleanName = FPaths::GetCleanFilename(ShotFile);

					if (ShotFileCleanName.MatchesWildcard(naming.Key))
					{
						bIsValidNaming = true;
					}

				}

				if (!bIsValidNaming)
				{
					continue;
				}

				//Create FShotData for return
				TSharedPtr<FShotData> CurrentShotData = MakeShared<FShotData>();

				FString ShotName = FPaths::GetBaseFilename(ShotListPath);
				FString ShotPrefix = ShotName.Left(3);
				FString FileBaseName = FPaths::GetBaseFilename(ShotFile);
				FString ContentAssetDirPath = FPaths::Combine(ContentShotRootPath, ShotPrefix, ShotName);
				FString ContentAssetFilePath = FPaths::Combine(ContentShotRootPath, ShotPrefix, ShotName, FileBaseName);

				FString LastestVersionFolder = LastestVersionPath.Right(3);
				int32 LastestVersion = FCString::Atoi(*LastestVersionFolder);

				FString CurrentImportPath = Utility::GetAssetImportPath(ContentAssetFilePath);
				FString CurrentVersionFolder = FPaths::GetPath(CurrentImportPath).Right(3);
				int32 CurrentVersion = FCString::Atoi(*CurrentVersionFolder);


				FString ShortenPath = CurrentImportPath;
				FString BasePath = ShotRootPath;

				if (!CurrentImportPath.IsEmpty())
				{
					FPaths::NormalizeFilename(ShortenPath);
					FPaths::NormalizeFilename(BasePath);
					FPaths::MakePathRelativeTo(ShortenPath, *BasePath);
				}

				FString GetPolishStateText;

				// Get Asset Staus
				bool bIsAssetExists = UEditorAssetLibrary::DoesAssetExist(ContentAssetFilePath);

				if (bIsAssetExists)
				{
					if (CurrentVersion == LastestVersion)
					{
						CurrentShotData->AssetStatus = EAssetStatus::UpToDate;
					}
					else if (CurrentVersion < LastestVersion)
					{
						CurrentShotData->AssetStatus = EAssetStatus::Loaded;
					}
				}
				else
				{
					CurrentShotData->AssetStatus = EAssetStatus::Unloaded;
				}

				// Update FShotData Attribute
				// Shot Info
				CurrentShotData->ShotName = ShotName;
				CurrentShotData->ShotMainName = ShotPrefix;

				// Current Asset Data
				CurrentShotData->AssetName = FileBaseName;
				CurrentShotData->CurrentAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(ContentAssetFilePath));
				CurrentShotData->ContentAssetDirPath = ContentAssetDirPath;
				CurrentShotData->ContentAssetFilePath = ContentAssetFilePath;
				CurrentShotData->CurrentVersion = CurrentVersion;
				CurrentShotData->CurrentImportPathShorten = ShortenPath;

				// Lastest in Directory Info
				CurrentShotData->LastestFilePath = ShotFile;
				CurrentShotData->LastestVersion = LastestVersion;

				// Create Polish Text
				if (CurrentShotData->AssetStatus == EAssetStatus::Unloaded)
				{
					GetPolishStateText = FString::Printf(TEXT("%d"), LastestVersion);
				}
				else if (CurrentShotData->AssetStatus == EAssetStatus::Loaded)
				{
					GetPolishStateText = FString::Printf(TEXT("%d < %d >"), LastestVersion, CurrentVersion);
				}
				else if (CurrentShotData->AssetStatus == EAssetStatus::UpToDate)
				{
					GetPolishStateText = FString::Printf(TEXT("%d"), LastestVersion);
				}

				// Create Polish State text
				CurrentShotData->PolishStateText = GetPolishStateText;

				// Add to Array
				ShotDataListResult.Add(CurrentShotData);

				// Debugig
				if (false) {
					UE_LOG(LogTemp, Log, TEXT("# Shot File Found : %s"), *ShotFile);
					UE_LOG(LogTemp, Log, TEXT("- Base Path : %s , Full Path : %s , ShortenPath : %s"), *BasePath, *ShotFile, *ShortenPath);
					UE_LOG(LogTemp, Log, TEXT("- Current Import Path : %s"), *CurrentImportPath);
					UE_LOG(LogTemp, Log, TEXT("- Content Browser Asset Path : %s"), *ContentAssetFilePath);
				}

			}
		}
	}
	return ShotDataListResult;
}
}