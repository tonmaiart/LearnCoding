// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ShotReader.generated.h"


typedef TMap<FString, FString> FNamingTypeMap;

USTRUCT(BlueprintType) struct FShotData
{
	GENERATED_BODY()

	// Status
	UPROPERTY() bool IsAnyVersionExists;
	UPROPERTY() bool IsAssetExists;

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

};

class SShotReaderWidgetTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SShotReaderWidgetTab){}

	SLATE_ARGUMENT(TArray<TSharedPtr<FShotData>>,ShotDataList)
	SLATE_ARGUMENT(FNamingTypeMap, namingTypes)
	
	SLATE_END_ARGS()

	private:
		FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

	public:

		TSharedPtr<SListView<TSharedPtr<FShotData>>> ShotListView;
		TArray<TSharedPtr<FShotData>> ShotDataList;
		TArray<TSharedPtr<FShotData>> ShotDataListBase;

		FString ShotRootPath;

		TMap<FString, FString> namingTypes;
		TArray<TSharedPtr<FString>> ListComboBoxTypeFilter;
		TSharedPtr<FString> SelectedFilterType;

		TArray<TSharedPtr<FString>> SequenceNameList;
		TSharedPtr<FString> SelectedSequenceName;

		#pragma region ConstructWidget
		void Construct(const FArguments& InArgs);

		TSharedRef<SListView<TSharedPtr<FShotData>>> ConstructAssetListView();
		TSharedPtr< SListView <TSharedPtr <FShotData>>> ConstructedAssetListView;

		//TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);
	
		TSharedRef<ITableRow> OnGeneratedRowAssetList (TSharedPtr<FShotData> ShotDataStruct,const TSharedRef<STableViewBase>& OwnerTable);
		TSharedPtr<SWidget> OnGeneratedContextMenu();
		TSharedRef<SWidget> OnGenerateComboFilterTypeWidget(TSharedPtr<FString> InItem);
		TSharedRef<SWidget> OnGenerateComboSequenceName(TSharedPtr < FString> InItem);

		void OnComboSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
		void OnComboSequenceSelectionChanged(TSharedPtr < FString> NewSelection, ESelectInfo::Type SelectInfo);

		void ReimportSelectedItem();
		void ImportSelectedItem();
		void BrowseFileLocation();
		void BrowseAssetLocation();
		void BuildSequencerToSelectedShot();
		void ReloadAll();
		void UpdateShotListFilter();
		void UpdateShotListFilterWidget();

		FReply OnReloadButtonClicked();

		#pragma endregion
};

