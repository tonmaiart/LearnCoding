// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ShotReader.generated.h"

USTRUCT(BlueprintType) struct FShotData
{
	GENERATED_BODY()

	UPROPERTY() FString ShotMainName;
	UPROPERTY() FString ShotName;
	UPROPERTY() int32 ShotVersion;
	UPROPERTY() FString LastestFilePath;
	UPROPERTY() FString CurrentImportPath;

	UPROPERTY() TArray<FString> ExternalFileNameList;
	UPROPERTY() TMap<FString,FAssetData> CurrentAssetDataList;

};

class SShotReaderWidgetTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SShotReaderWidgetTab){}

	SLATE_ARGUMENT(TArray <TSharedRef<FShotData>>,ShotDataList)
	
	SLATE_END_ARGS()

private:
	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

public:

TArray<TSharedRef<FShotData>> ShotDataList;
FString ShotRootPath;

#pragma region ConstructWidget
void Construct(const FArguments& InArgs);
TSharedRef<SListView<TSharedRef<FShotData>>> ConstructAssetListView();
TSharedRef<ITableRow> OnGeneratedRowAssetList (TSharedRef<FShotData> ShotDataStruct,const TSharedRef<STableViewBase>& OwnerTable);

#pragma endregion
};

