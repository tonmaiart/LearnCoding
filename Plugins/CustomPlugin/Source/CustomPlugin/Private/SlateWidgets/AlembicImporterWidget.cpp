// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AlembicImporterWidget.h"
#include "Widgets/Input/SSlider.h"
#include "SlateBasics.h"
#include "DebugHeader.h"

void SAlembicImporterWidgetTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	StoredAssetData = InArgs._AssetDataToStore;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

	ChildSlot
		[
			// Main Vertical Box
			SNew(SVerticalBox)

			// First Vertical Slot for title text
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Alembic Importer")))
					.Font(TitleTextFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FColor::White)
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Alembic Importer")))
						.Font(TitleTextFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSlider)
						//.Style(USlateWidgetStyleAsset::GetStyle())
				]

			// Last block store horizontal
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

				]

			// Last block store horizontal
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)

				+ SScrollBox::Slot()
					[
						SNew(SListView<TSharedPtr <FAssetData>>)
							.ItemHeight(24.f)
							.ListItemsSource(&StoredAssetData)
							.OnGenerateRow(this,&SAlembicImporterWidgetTab::OnGenerateRowForList)

					]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

			]

		];


}

TSharedRef<ITableRow> SAlembicImporterWidgetTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Return if Input Data is Valid
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow < TSharedPtr <FAssetData>>, OwnerTable);


	const FString DisplayAssetname = AssetDataToDisplay->AssetName.ToString();

	TSharedRef < STableRow <TSharedPtr <FAssetData>>> ListViewRowWidget =
		SNew(STableRow < TSharedPtr <FAssetData>>, OwnerTable)

		[
			// Horizontal Box
			SNew(SHorizontalBox)
			

			// Slot 1 : Check Box
			+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(.05F)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]
			// Slot 2 :

			// Slot 3 : Text of Asset name
			+SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromString(DisplayAssetname))
				]

		];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAlembicImporterWidgetTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAlembicImporterWidgetTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	return ConstructedCheckBox;
}

void SAlembicImporterWidgetTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	DebugHeader::Print(AssetData->AssetName.ToString() + StaticEnum<ECheckBoxState>()->GetNameStringByValue((int64)NewState ), FColor::Green);

}