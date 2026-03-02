// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UkoreToolkitSettings.generated.h"

UCLASS(Config = Editor)
class UKORETOOLKIT_API UUkoreToolkitSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	//UCustomPluginSettings();
	UPROPERTY(Config, DisplayName = "All Naming Types", EditAnywhere, Category = "Asset Manager")
	TMap<FString, FString> namingTypes;

	UPROPERTY(Config, DisplayName = "Root Path in Absolute Diretory", EditAnywhere, Category = "Asset Manager")
	TArray<FString> ShotRootPaths;

	UPROPERTY(Config, DisplayName = "Sub Folder Name", EditAnywhere, Category = "Asset Manager")
	FString SubFolder = "Layout";

	UPROPERTY(Config, DisplayName = "Root Path in Content Browser", EditAnywhere, Category = "Asset Manager")
	FString ContentShotRootPath = "/Game/Sequences";

};
