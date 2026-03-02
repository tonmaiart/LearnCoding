// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ShotData.generated.h"

UENUM(BlueprintType)
enum class EAssetStatus : uint8
{
	Loaded        UMETA(DisplayName = "Loaded"),
	Unloaded   UMETA(DisplayName = "Unloaded"),
	UpToDate      UMETA(DisplayName = "UpToDate"),
};

USTRUCT(BlueprintType) struct FShotData
{
	GENERATED_BODY()

	// Status
	UPROPERTY() bool IsAnyVersionExists;
	UPROPERTY() EAssetStatus AssetStatus;

	// Common Info
	UPROPERTY() FString ShotMainName;
	UPROPERTY() FString ShotName;
	UPROPERTY() int32 CurrentVersion;
	UPROPERTY() int32 LastestVersion;

	UPROPERTY() FString LastestFilePath;
	UPROPERTY() FString PolishStateText;

	UPROPERTY() FString AssetName;
	UPROPERTY() FString CurrentImportPathShorten;
	UPROPERTY() FString CurrentImportPath;
	UPROPERTY() FString ContentAssetDirPath;
	UPROPERTY() FString ContentAssetFilePath;

	UPROPERTY() TArray<FString> ExternalFileNameList;
	UPROPERTY() FAssetData CurrentAssetData;

	UPROPERTY() FString FileType;

};