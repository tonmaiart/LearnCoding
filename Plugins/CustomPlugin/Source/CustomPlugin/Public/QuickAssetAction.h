// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Materials/MaterialInstanceConstant.h"
#include "QuickAssetAction.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMPLUGIN_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor)
	void TestFunc();

	UFUNCTION(CallInEditor)
	void AutoAssignMaterial();

	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumOfDuplicates);

	UFUNCTION(CallInEditor)
	void AddPrefixes();

	UFUNCTION(CallInEditor)
	void RemoveUnusedAsset();

private:

	TMap<UClass*, FString>PrefixMap = 

	{
		{UBlueprint::StaticClass(),TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(),TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")}
	};
};
