// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
//#include "UkoreAssetImporter.generated.h"
#include "UkoreCore/ShotData.h"




namespace AssetImporterUtility
{
//static void ReimportSelectedItem();
//static void ImportSelectedItem();
//static void BrowseFileLocation();
//static void BrowseAssetLocation();
//static void BuildSequencerToSelectedShot();
//static void ReloadAll();

UObject* CreateNewSequencerFromTemplate(FString DirNewSequencerPath, FString NewSequencerName, FString SequencerTemplatePath = FString("/UkoreToolkit/Templates/SEQ_TEMPLATE.SEQ_TEMPLATE"));
bool ImportCameraToSequencer(UObject* NewSequenceObject, const FString& NewSequencerPath, const FString& CameraImportPath, const FString CameraName = "RenderCam");
bool AddAlembicToSequencer(FString SequencerPath, FString AlembicAssetPath, FString ActorLabel);

// Use For Get All Need Data for update shot
TArray<TSharedPtr<FShotData>> GetShotDataArray(
	TArray<FString> ShotRootPaths,
	FString SubFolder,
	TMap<FString,FString>namingTypes,
	FString ContentShotRootPath); 

}
