// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CustomPluginSettings.generated.h"

/**
 * 
 */
UCLASS(Config= Editor)
class CUSTOMPLUGIN_API UCustomPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	//UCustomPluginSettings();
	UPROPERTY(Config, DisplayName = "All Naming Types", EditAnywhere, Category = "Shot Reader")
	TMap<FString, FString> namingTypes;

	UPROPERTY(Config, DisplayName = "Root Path in Absolute Diretory", EditAnywhere, Category = "Shot Reader")
	TArray<FString> ShotRootPaths;

	UPROPERTY(Config, DisplayName = "Sub Folder Name", EditAnywhere, Category = "Shot Reader")
	FString SubFolder = "Layout";

	UPROPERTY(Config, DisplayName = "Root Path in Content Browser", EditAnywhere, Category = "Shot Reader")
	FString ContentShotRootPath = "/Game/Sequences";



};
