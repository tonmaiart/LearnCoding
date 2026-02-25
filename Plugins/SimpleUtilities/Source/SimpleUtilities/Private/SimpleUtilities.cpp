// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleUtilities.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "CoreMinimal.h"

#define LOCTEXT_NAMESPACE "FSimpleUtilitiesModule"

void FSimpleUtilitiesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSimpleUtilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FSimpleUtilitiesModule::IsPathSecure(const FString& AssetPath)
{
	// Don't touch root folder
	if (AssetPath.Contains(TEXT("Developers"))
		|| AssetPath.Contains(TEXT("Collections"))
		|| !UEditorAssetLibrary::DoesAssetExist(AssetPath))
	{
		return false;
	}
	else
	{
		return true;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSimpleUtilitiesModule, SimpleUtilities)