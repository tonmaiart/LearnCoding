// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UkoreCore/ShotData.h"

typedef TMap<FString, FString> FNamingTypeMap;


class SUkoreAssetImporterWidgetTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SUkoreAssetImporterWidgetTab) {}
	SLATE_END_ARGS()

private:
	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

public:
	// Main Data
	TArray<TSharedPtr<FShotData>> ShotDataList;
	TArray<TSharedPtr<FShotData>> ShotDataListBase;

	// Configuration from Developer Settings
	TArray<FString> ShotRootPaths;
	FString SubFolder;
	FString ContentShotRootPath;
	TMap<FString, FString> namingTypes;

	// Widget Variables
	TArray<TSharedPtr<FString>> ListComboBoxTypeFilter;
	TSharedPtr<FString> SelectedFilterType;
	TArray<TSharedPtr<FString>> SequenceNameList;
	TSharedPtr<FString> SelectedSequenceName;
	TSharedPtr<SListView<TSharedPtr<FShotData>>> ShotListView;



#pragma region ConstructWidget
	void Construct(const FArguments& InArgs);

	TSharedRef<SListView<TSharedPtr<FShotData>>> ConstructAssetListView();
	TSharedPtr< SListView <TSharedPtr <FShotData>>> ConstructedAssetListView;

	//TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);

	TSharedRef<ITableRow> OnGeneratedRowAssetList(TSharedPtr<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedPtr<SWidget> OnGeneratedContextMenu();
	TSharedRef<SWidget> OnGenerateComboFilterTypeWidget(TSharedPtr<FString> InItem);
	TSharedRef<SWidget> OnGenerateComboSequenceName(TSharedPtr < FString> InItem);

	void OnComboSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnComboSequenceSelectionChanged(TSharedPtr < FString> NewSelection, ESelectInfo::Type SelectInfo);

	void ReloadAll();
	void UpdateShotListFilter();
	void UpdateShotListFilterWidget();
	void FetchDataForListComboBox();
	void FetchConfiguration();
	void FetchExistShotDataList();

	TArray<TSharedPtr<FShotData>> GetShotData(); // Use For Get All Need Data for update shot
	TArray<TSharedPtr<FShotData>> GetSelectedItemListView();

	// Clicked Action
	FReply OnReloadButtonClicked();
	void OnImportSelectedItemClicked();
	void OnReimportSelectedItemClicked();
	void OnBrowseFileLocationClicked();
	void OnBrowseAssetLocation();
	void OnBuildSequencerToSelectedShot();

#pragma endregion
};