// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAlembicImporterWidgetTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAlembicImporterWidgetTab){}
	
	SLATE_ARGUMENT(TArray<TSharedPtr <FAssetData> >, AssetDataToStore)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
private:
	TArray<TSharedPtr<FAssetData>> StoredAssetData;
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	void OnCheckBoxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData);
};