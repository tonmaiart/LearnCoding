// Fill out your copyright notice in the Description page of Project Settings.

#include "UkoreAssetImporter/UkoreAssetImporter.h"
#include "UkoreAssetImporter/UkoreAssetImporterWidgetTab.h"

#include "UkoreToolkit.h"
#include "UkoreCore/UkoreUtility.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"

#include "Modules/ModuleManager.h"
#include "IAssetTools.h"

#include "GeometryCache.h"
#include "GeometryCacheActor.h"
#include "GeometryCacheComponent.h"

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
#include "MovieSceneGeometryCacheTrack.h"
#include "MovieSceneGeometryCacheSection.h"
#include "GeometryCacheTracksModule.h"


#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "EditorReimportHandler.h"

namespace AssetImporterUtility{
	bool ImportCameraToSequencer(UObject* NewSequenceObject, const FString& NewSequencerPath,const FString& CameraImportPath,const FString CameraName)
	{
		
		ISequencer* SequencerInstance = nullptr;

		// Import Camera
		if (!CameraImportPath.IsEmpty()) {
			UE_LOG(LogTemp, Log, TEXT("Import Camera Target : %s"), *CameraImportPath);

			// Open Sequencer
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewSequenceObject);

			// Retrieve Sequencer Pointer
			TSharedPtr<ISequencer> SequencerPtr;

			if (IAssetEditorInstance* EditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(NewSequenceObject, false))
			{
				ILevelSequenceEditorToolkit* LevelSequenceToolkit = static_cast<ILevelSequenceEditorToolkit*>(EditorInstance);

				if (LevelSequenceToolkit)
				{
					SequencerPtr = LevelSequenceToolkit->GetSequencer();
				}
			}

			// Create Main Variable
			ULevelSequence* NewLevelSequence = Cast<ULevelSequence>(NewSequenceObject);
			UMovieScene* MovieScene = NewLevelSequence->GetMovieScene();

			// Get Camera FFUID
			FGuid CameraGuid = Utility::GetCameraGuidByName(NewLevelSequence, TEXT("RenderCam"));

			if (!CameraGuid.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Import Camera Failed , Camera Guid is Invalid"));
				return false;
			}

			// Create Import Settings
			UMovieSceneUserImportFBXSettings* ImportSettings = NewObject<UMovieSceneUserImportFBXSettings>();
			ImportSettings->bReplaceTransformTrack = true; // Essential for mapping to existing Guid
			ImportSettings->bCreateCameras = false;        // Since you are mapping to "RenderCam"
			ImportSettings->bReduceKeys = true;

			FFBXInOutParameters OutFBXParams;

			// Check is Ready to Import Fbx
			bool bIsReady = MovieSceneToolHelpers::ReadyFBXForImport(CameraImportPath, ImportSettings, OutFBXParams);

			if (!bIsReady)
			{
				UE_LOG(LogTemp, Log, TEXT("- Camera Not Ready FBX For Import"));
				return false;
			}


			TMap<FGuid, FString> ObjectBindingMap;
			ObjectBindingMap.Add(CameraGuid, TEXT("RenderCam"));

			// Get World
			UWorld* World = GEditor->GetEditorWorldContext().World();
			if (!World) {
				UE_LOG(LogTemp, Error, TEXT("World is null!"));
				return false;
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("World is ready : %s"), *(World->GetMapName()));
			}

			// Guard Checks
			if (MovieScene == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Sequence has no MovieScene data!"));
				UE_LOG(LogTemp, Log, TEXT("Movie Scene is invalid "));
				return false;

			}

			// Import FBX Camera

			if (SequencerPtr.IsValid())
			{

				MovieSceneToolHelpers::ImportFBXIfReady(
					World,
					NewLevelSequence,
					SequencerPtr.Get(),
					SequencerPtr->GetRootTemplateID(),
					ObjectBindingMap,
					ImportSettings,
					OutFBXParams
				);
				UE_LOG(LogTemp, Log, TEXT("Re-Import Camera Complete!"));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Invalid SequencerPtr"));
				return false;

			}
		}

		else
		{
			return false;
		}
	}
	bool AddAlembicToSequencer(FString SequencerPath, FString AlembicAssetPath, FString ActorLabel)
	{
		// Get Level Sequence
		//UObject* SequencerObject = ;
		ULevelSequence* LevelSequence = Cast<ULevelSequence>(UEditorAssetLibrary::LoadAsset(SequencerPath));

		if (!LevelSequence)
		{
			return false;
		}

		// Get World
		UWorld* World = LevelSequence->GetWorld();

		if (!World)
		{
			return false;
		}

		// Get Movie Scene

		UMovieScene* MovieScene = LevelSequence->GetMovieScene();
		if (!MovieScene)
		{
			return false;
		}

		// Add Alembic to Sequence
		UGeometryCache* GeoCache = Cast<UGeometryCache>(UEditorAssetLibrary::LoadAsset(AlembicAssetPath));

		if (!GeoCache)
		{
			return false;
		}

		AGeometryCacheActor* GeoActor = World->SpawnActor<AGeometryCacheActor>(AGeometryCacheActor::StaticClass(), FTransform::Identity);

		if (!GeoActor)
		{
			UE_LOG(LogTemp,Log,TEXT("Failed to spawn Geometry Cache Actor"))
		}

		GeoActor->SetActorLabel(ActorLabel);

		UGeometryCacheComponent* GeoComponent = GeoActor->GetGeometryCacheComponent();

		if (GeoComponent)
		{
			GeoComponent->SetGeometryCache(GeoCache);
			GeoComponent->RegisterComponent();
		}

		// Connect BInding
		FGuid ActorGuid = MovieScene->AddPossessable(ActorLabel, GeoActor->GetClass());
		LevelSequence->BindPossessableObject(ActorGuid, *GeoActor,World);

		// Add Geometry Cache Track to binding
		UGeometryCacheTrack* GeoCacheTrack = MovieScene->AddTrack<UGeometryCacheTrack>(ActorGuid);

		if (!GeoCacheTrack)
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to add GeometryCacheTrack"));
			return false;
		}

		// Add a section to the track
		FFrameRate TickResolution = MovieScene->GetTickResolution();
		FFrameRate DisplayRate = MovieScene->GetDisplayRate();

		FFrameNumber StartFrame = MovieScene->GetPlaybackRange().GetLowerBoundValue();
		FFrameNumber EndFrame = MovieScene->GetPlaybackRange().GetUpperBoundValue();

		UMovieSceneGeometryCacheSection* GeoCacheSection = Cast<UMovieSceneGeometryCacheSection>(GeoCacheTrack->CreateNewSection());

		if (GeoCacheSection)
		{
			GeoCacheSection->SetRange(TRange<FFrameNumber>(StartFrame, EndFrame));

			FMovieSceneGeometryCache
		}


		return false;
	}
	UObject* CreateNewSequencerFromTemplate(FString DirNewSequencerPath, FString NewSequencerName, FString SequencerTemplatePath)
	{
		// Do not create sequencer if asset path already exist
		FString NewSequencerPath = FPaths::Combine(DirNewSequencerPath, NewSequencerName);

		if (UEditorAssetLibrary::DoesAssetExist(NewSequencerPath))
		{
			UE_LOG(LogTemp, Log, TEXT("Sequencer %s already exist, cannot create new sequencer."),*NewSequencerPath);
			return nullptr;
		}

		// Create new sequencer
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UObject* SequenceTemplate = UEditorAssetLibrary::LoadAsset(SequencerTemplatePath);
		UObject* NewSequenceObject = AssetTools.DuplicateAsset(NewSequencerName, DirNewSequencerPath, SequenceTemplate);

		// Return Result
		if (!NewSequenceObject)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to duplicate Sequence Template!"));
			return nullptr;
		}
		else
		{
			return NewSequenceObject;
		}

	}
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


				//Create FShotData for return
				TSharedPtr<FShotData> CurrentShotData = MakeShared<FShotData>();

				// Check is Valid Naming Convention
				for (const TPair<FString, FString> naming : namingTypes)
				{
					FString ShotFileCleanName = FPaths::GetCleanFilename(ShotFile);

					if (ShotFileCleanName.MatchesWildcard(naming.Key))
					{
						bIsValidNaming = true;
						CurrentShotData->FileType = naming.Value;

					}

				}

				if (!bIsValidNaming)
				{
					continue;
				}


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

