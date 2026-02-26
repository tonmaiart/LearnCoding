// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IAssetTools.h"

#include "GeometryCache.h"
#include "AlembicImportFactory.h"
#include "AbcImportSettings.h"
#include "CoreMinimal.h"

#include "EditorAssetLibrary.h"



#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "MovieScene.h"
#include "MovieSceneToolHelpers.h"
#include "MovieScenePossessable.h"
#include "MovieSceneToolsUserSettings.h"

#include "LevelSequenceEditorModule.h"
#include "Subsystems/AssetEditorSubsystem.h"

//#include "/*Factories*//LevelSequenceFactoryNew.h"
//#include "LevelSequenceFactoryNew.h"

class ULevelSequence;

class UGeometryCache;

class FSimpleUtilitiesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsPathSecure(const FString& AssetPath);
};


namespace Utility
{ 
	struct FVersionResult
	{
		FString Path = "";
		int32 Version = 0;

	};

	static TArray<FString> GetDirectoryContent(FString DirectoryPath, bool GetDir, bool GetFile)
{
	// Prepare the output array
	TArray<FString> FoundList;
	TArray<FString> ResultList;

	// Define the visitor
	struct FLocalVisitor : public IPlatformFile::FDirectoryVisitor
	{
		TArray<FString>& OutArray;
		FLocalVisitor(TArray<FString>& InArray) : OutArray(InArray) {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			OutArray.Add(FilenameOrDirectory);
			return true;
		}
	};

	// Execute Search
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalVisitor Visitor(FoundList);
	PlatformFile.IterateDirectory(*DirectoryPath, Visitor);

	// Filter
	for (const FString item : FoundList)
	{
		if (PlatformFile.DirectoryExists(*item) && GetDir == true)
		{
			ResultList.Add(item);
		}
		else if (PlatformFile.FileExists(*item) && GetFile == true)
		{
			ResultList.Add(item);
		}
	}

	return ResultList;
}
	
	/* Get Lastest Folder of Given Directory Path */
	static FVersionResult GetLastestVersionFolder(const FString ParentPath)
	{
		TArray<FString> AllSubDirs = GetDirectoryContent(ParentPath, true, false);
		TArray<FString> VersionFolder;
		TArray<int32> AllVersion;

		FRegexPattern Pattern(TEXT("^v\\d{3}$"));

		for (const FString& FullPath : AllSubDirs)
		{
			// Get only folder name from path
			FString FolderName = FPaths::GetCleanFilename(FullPath);

			FRegexMatcher Matcher(Pattern, FolderName);

			if (Matcher.FindNext())
			{
				// Get Path
				VersionFolder.Add(FullPath);
				
				// Get Version
				FString VersionString = Matcher.GetCaptureGroup(1);
				AllVersion.Add(FCString::Atoi(*VersionString));
			}
		}
		
		FVersionResult ReturnResult;

		// Return Empty String if nothing found
		if (VersionFolder.Num() == 0)
		{
			return ReturnResult;
		}
		else
		{
			VersionFolder.Sort();
			AllVersion.Sort();

			ReturnResult.Path = VersionFolder.Last();
			ReturnResult.Version = AllVersion.Last();
			return ReturnResult;
		}
	}

	


	static UGeometryCache* ImportAlembicFile(FString SourceFilePath, FString DestinationPath)
		{
			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

			// Create AbcFactory
			UAlembicImportFactory* AbcFactory = NewObject<UAlembicImportFactory>();

			//Setting Import Configuration
			AbcFactory->ImportSettings->ImportType = EAlembicImportType::GeometryCache;
			AbcFactory->ImportSettings->GeometryCacheSettings.bFlattenTracks = true;

			//Close Ui (Slient Import)
			AbcFactory->bEditAfterNew = false;
			AbcFactory->bShowOption = false;

			// Import Automate
			TArray<FString> FileNames;
			FileNames.Add(SourceFilePath);

			UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
			ImportData->Filenames.Add(SourceFilePath);
			ImportData->DestinationPath = DestinationPath;

			TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

			for (UObject* asset : ImportedAssets) {
				UEditorAssetLibrary::SaveAsset(asset->GetPathName(), false);
			}



			return nullptr;

		}

	static FString GetAssetImportPath(FString AssetFullPath)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetFullPath));

		if (AssetData.IsValid())
		{
			FString ImportPath;

			if (AssetData.GetTagValue(TEXT("AssetImportData"), ImportPath))
			{
				UE_LOG(LogTemp, Log, TEXT("Get Source Path"), *ImportPath);
				return ImportPath;
			}
		}

		return FString();
	}

	static FGuid GetCameraGuidByName(ULevelSequence* InSequence, FString CameraName)
	{
		if (!InSequence) return FGuid();

		UMovieScene* MovieScene = InSequence->GetMovieScene();

		for (int32 i = 0; i < MovieScene->GetPossessableCount(); ++i)
		{
			const FMovieScenePossessable& Possessable = MovieScene->GetPossessable(i);

			if (Possessable.GetName() == CameraName)
			{
				return Possessable.GetGuid();
			}
		}

		return FGuid();
	}

	static UObject* BuildSequencer(FString NewSequencerPath ,
		TArray<FString> AlembicCachePaths = TArray<FString>(),
		FString CameraImportPath = FString(),
		TArray<FString> ToonshadeAlembicCachePaths = TArray<FString>())
	{
		UE_LOG(LogTemp, Log, TEXT("# Create New Sequencer Asset : %s"),*NewSequencerPath);

		// Do not create sequencer if asset path already exist
		if (UEditorAssetLibrary::DoesAssetExist(NewSequencerPath))
		{
			UE_LOG(LogTemp, Log, TEXT("Sequencer already exist, cannot build."));
			return nullptr;
		}
		
		// Create new sequencer
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		FString SequenceName = FPaths::GetBaseFilename(FString(NewSequencerPath));
		FString DirSequencePath = FPaths::GetPath(FString(NewSequencerPath));
		UObject* SequenceTemplate = UEditorAssetLibrary::LoadAsset(FString("/SimpleUtilities/Template/SeqTemplate.SeqTemplate"));
		UObject* NewSequence =  AssetTools.DuplicateAsset(SequenceName, DirSequencePath, SequenceTemplate);

		if (!NewSequence)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to duplicate Sequence Template!"));
			return nullptr;
		}

		ULevelSequence* NewSequenceAsset = Cast<ULevelSequence>(NewSequence);

		// Import Camera
		if (!CameraImportPath.IsEmpty()) {
			UE_LOG(LogTemp, Log, TEXT("Import Camera Target : %s"),*CameraImportPath);

			FGuid CameraGuid = GetCameraGuidByName(NewSequenceAsset,TEXT("RenderCam"));

			if (CameraGuid.IsValid())
			{
				UMovieScene* MovieScene = NewSequenceAsset->GetMovieScene();

				// Create Import Settings
				UMovieSceneUserImportFBXSettings* ImportSettings = NewObject<UMovieSceneUserImportFBXSettings>();
				ImportSettings->bReplaceTransformTrack = true; // Essential for mapping to existing Guid
				ImportSettings->bCreateCameras = false;        // Since you are mapping to "RenderCam"
				ImportSettings->bReduceKeys = true;

				FFBXInOutParameters OutFBXParams;
				bool bIsReadey = MovieSceneToolHelpers::ReadyFBXForImport(CameraImportPath, ImportSettings, OutFBXParams);
				
				if (bIsReadey) {
					UE_LOG(LogTemp, Log, TEXT("ReadyFBXForImport"));

					TMap<FGuid, FString> ObjectBindingMap;
					ObjectBindingMap.Add(CameraGuid, TEXT("RenderCam"));

					UWorld* World = GEditor->GetEditorWorldContext().World();

					if (!World) {
						UE_LOG(LogTemp, Error, TEXT("World is null!"));
						return nullptr;
					}

					else
					{
						UE_LOG(LogTemp, Log, TEXT("World is ready : %s"),*(World->GetMapName()));

					}

					// Guard Checks
					if (MovieScene == nullptr)
					{
						UE_LOG(LogTemp, Error, TEXT("Sequence has no MovieScene data!"));
						UE_LOG(LogTemp, Log, TEXT("Movie Scene is invalid "));

					}

					if (NewSequenceAsset)
					{
						GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()
							->OpenEditorForAsset(NewSequenceAsset);

					}

					MovieSceneToolHelpers::ImportFBXIfReady(
						World,
						NewSequenceAsset,
						nullptr,
						FMovieSceneSequenceID(0),
						ObjectBindingMap,
						ImportSettings,
						OutFBXParams
					);
				}

				UE_LOG(LogTemp, Log, TEXT("Re-Import Camera Complete!"));

			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Import Camera Failed"));

			}
		}

		return NewSequence;

	}

	static bool UpdateCameraInSequencer()
	{
	}



		
	}


	