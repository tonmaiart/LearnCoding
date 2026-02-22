// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ObjectTools.h"

void UQuickAssetAction::TestFunc()
{
	DebugHeader::Print(TEXT("WORKING"), FColor::Green);
	DebugHeader::PrintLog(TEXT("WORKING"));
}

void UQuickAssetAction::AutoAssignMaterial()
{
}

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	DebugHeader::Print(TEXT("Run Duplicate Assets"), FColor::Green);

	//Guard Check
	if (NumOfDuplicates <= 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please Enter Valid Number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.GetObjectPathString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;

			}

		}
	}

	if (Counter > 0) {
		DebugHeader::Print(TEXT("Test"), FColor::Green);
		DebugHeader::ShowNotifyInfo(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " files"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	TArray<UObject*> AssetsToReselect;

	uint32 SelectedLen = SelectedAssets.Num();
	uint32 Counter = 0;
	DebugHeader::Print(TEXT("Selected Length :") + FString::FromInt(SelectedLen), FColor::Green);

	for (UObject* SelAssetObj : SelectedAssets)
	{
		if (!SelAssetObj) continue;

		UObject* SelAssetObjClass = SelAssetObj->GetClass();

		// Try to Cast blueprint type and check is it bluepinrt interface
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(SelAssetObj);


		if (BlueprintAsset && (BlueprintAsset->BlueprintType == EBlueprintType::BPTYPE_Interface))
		{
			DebugHeader::Print(TEXT("This is Blueprint Interface"), FColor::White);
		}

		// Ignore Empty
		FString* PrefixFound = PrefixMap.Find(SelAssetObj->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class") + SelAssetObj->GetName(), FColor::Red);
			continue;
		}


		FString OldName = SelAssetObj->GetName();

		// Remove _inst from Material Instance
		if (SelAssetObj->IsA<UMaterialInstanceConstant>())
		{
			DebugHeader::Print(TEXT("Removed M_ , _Inst") + SelAssetObj->GetName(), FColor::Red);

			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromStart(TEXT("MI_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}


		// Check Prefix
		if (OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		// Add Prefix
		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelAssetObj, NewNameWithPrefix);
		AssetsToReselect.Add(SelAssetObj);

		++Counter;
	}


	if (Counter > 0) {
		DebugHeader::ShowNotifyInfo(FString(TEXT("Successfully renamed ") + FString::FromInt(Counter) + TEXT("assets")));
		DebugHeader::Print(TEXT("OUPTUT") + SelectedAssets[0]->GetName(), FColor::White);
	}

	// Keep Select 
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();

	ContentBrowserSingleton.SyncBrowserToAssets(AssetsToReselect);

}

void UQuickAssetAction::RemoveUnusedAsset()
{
	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetData;

	for (const FAssetData AssetData : SelectedAssetData)
	{
		TArray<FString> ReferencesAsset = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.GetSoftObjectPath().ToString(), true);

		DebugHeader::Print(TEXT("This Asset Have references amount : ")+ FString::FromInt(ReferencesAsset.Num()), FColor::White);

		if (ReferencesAsset.Num() == 0)
		{
			UnusedAssetData.Add(AssetData);
		}
	}

	if (UnusedAssetData.Num() != 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetData);
	}

}
