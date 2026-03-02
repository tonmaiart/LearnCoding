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
#include "ILevelSequenceEditorToolkit.h"

#include "MovieScene.h"
#include "MovieSceneToolHelpers.h"
#include "MovieScenePossessable.h"
#include "MovieSceneToolsUserSettings.h"

#include "LevelSequenceEditorModule.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "EditorFramework/AssetImportData.h"

#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"


class ULevelSequence;

class UGeometryCache;


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

		ResultList.Sort();

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

	static FString GetAssetImportPath(const FString AssetFullPath)
	{

		UObject* Asset = LoadObject<UObject>(nullptr, AssetFullPath);

		if (Asset)
		{
			for (TFieldIterator<FObjectProperty> PropIt(Asset->GetClass()); PropIt; ++PropIt)
			{
				FObjectProperty* Prop = *PropIt;

				if (Prop->PropertyClass->IsChildOf(UAssetImportData::StaticClass()))
				{
					UAssetImportData* ImportData = Cast<UAssetImportData>(Prop->GetObjectPropertyValue_InContainer(Asset));
					if (ImportData)
					{
						FString SourceFilePath = ImportData->GetFirstFilename();

						return SourceFilePath;
					}
				}

			}
		}
		else
		{
			return FString();

		}

		return FString();

		//static FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		//FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetFullPath));

		//UE_LOG(LogTemp, Log, TEXT("- Getting Asset Import Path from: %s"), *AssetFullPath);

		//if (AssetData.IsValid())
		//{
		//	FString SerializedImportData;


		//	if (AssetData.GetTagValue(TEXT("AssetImportData"), SerializedImportData))
		//	{

		//		TSharedPtr<FAssetImportInfo> ImportInfo = FAssetImportInfo::FromJson(SerializedImportData);
		//		
		//		if (ImportInfo.IsValid() && ImportInfo ->SourceFiles.Num()>0)
		//		{
		//			FString ImportPath = ImportInfo->SourceFile[0].RelativesFilename;
		//			UE_LOG(LogTemp, Log, TEXT("- Get Re-Import Path :%s "), *ImportPath);
		//			return ImportPath;

		//		}

		//	}
		//	else
		//	{
		//		UE_LOG(LogTemp, Log, TEXT("- This Type of Asset dont have import path tag , Cannot get re-import path from : %s"), *AssetFullPath);
		//		return FString();
		//	}
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Log, TEXT("- Asset Data Invalid , Cannot get re-import path from : %s"), *AssetFullPath);
		//}


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

	static UObject* BuildSequencer(FString NewSequencerPath,
		TArray<FString> AlembicCachePaths = TArray<FString>(),
		FString CameraImportPath = FString(),
		TArray<FString> ToonshadeAlembicCachePaths = TArray<FString>())
	{
		UE_LOG(LogTemp, Log, TEXT("# Create New Sequencer Asset : %s"), *NewSequencerPath);

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
		UObject* SequenceTemplate = UEditorAssetLibrary::LoadAsset(FString("/UkoreToolkit/Templates/SEQ_TEMPLATE.SEQ_TEMPLATE"));
		UObject* NewSequenceObject = AssetTools.DuplicateAsset(SequenceName, DirSequencePath, SequenceTemplate);

		if (!NewSequenceObject)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to duplicate Sequence Template!"));
			return nullptr;
		}

		ISequencer* SequencerInstance = nullptr;

		// Import Camera
		if (!CameraImportPath.IsEmpty()) {
			UE_LOG(LogTemp, Log, TEXT("Import Camera Target : %s"), *CameraImportPath);

			// Open Sequencer
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewSequenceObject);

			// Retrieve Sequencer Pointer
			TSharedPtr<ISequencer> SequencerPtr;
			
			if (IAssetEditorInstance* EditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(NewSequenceObject,false))
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
			FGuid CameraGuid = GetCameraGuidByName(NewLevelSequence, TEXT("RenderCam"));

			if (!CameraGuid.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Import Camera Failed , Camera Guid is Invalid"));
				return nullptr;
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
				return nullptr;
			}


			TMap<FGuid, FString> ObjectBindingMap;
			ObjectBindingMap.Add(CameraGuid, TEXT("RenderCam"));

			// Get World
			UWorld* World = GEditor->GetEditorWorldContext().World();
			if (!World) {
				UE_LOG(LogTemp, Error, TEXT("World is null!"));
				return nullptr;
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
				return nullptr;

			}
			
			// Import FBX
			
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

			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Invalid SequencerPtr"));
				return nullptr;

			}


				
		}

		return NewSequenceObject;

	}

	static bool UpdateCameraInSequencer()
	{
	}




}

namespace DebugHeader
{

	static void Print(const FString& Message, const FColor& Color = FColor::White)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message)

	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true)
	{
		if (bShowMsgAsWarning == true) {
			return FMessageDialog::Open(MsgType, FText::FromString(Message), FText::FromString(TEXT("Warning")));

		}
		else
		{
			return FMessageDialog::Open(MsgType, FText::FromString(Message));
		}
	}

	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

}

