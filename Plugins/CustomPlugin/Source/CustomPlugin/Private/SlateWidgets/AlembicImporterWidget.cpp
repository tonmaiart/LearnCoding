// Fill out your copyright notice in the Description page of Project Settings.

#include "Modules/ModuleManager.h"
#include "SlateWidgets/AlembicImporterWidget.h"
#include "Widgets/Input/SSlider.h"
#include "SlateBasics.h"
#include "DebugHeader.h"
#include "CustomPlugin.h"


void SAlembicImporterWidgetTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	StoredAssetData = InArgs._AssetDataToStore;

	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 25;

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
						ConstructAssetListView()

					]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

			]

		];


}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAlembicImporterWidgetTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr <FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&StoredAssetData)
		.OnGenerateRow(this, &SAlembicImporterWidgetTab::OnGenerateRowForList);

	
	return ConstructedAssetListView.ToSharedRef();
}

void SAlembicImporterWidgetTab::RefreshAssetListView()
{
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

TSharedRef<ITableRow> SAlembicImporterWidgetTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Return if Input Data is Valid
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow < TSharedPtr <FAssetData>>, OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->GetName();
	const FString DisplayAssetname = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 15;
	AssetClassNameFont.TypefaceFontName = FName("Bold");

	TSharedRef < STableRow <TSharedPtr <FAssetData>>> ListViewRowWidget =
		SNew(STableRow < TSharedPtr <FAssetData>>, OwnerTable)
		.Padding(FMargin(4.f))

		[
			// Horizontal Box
			SNew(SHorizontalBox)
			

			// Slot 1 : Check Box
			+SHorizontalBox::Slot()
			[

				ConstructCheckBox(AssetDataToDisplay)
			]

			// Slot 2 : Text of Class name
			+ SHorizontalBox::Slot()
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
			]

			// Slot 3 : Text of Asset name
			+SHorizontalBox::Slot()
			[
				ConstructTextForRowWidget(DisplayAssetname, AssetNameFont)

			]

			// Slot 4 : Button for Focus to content browser
			+ SHorizontalBox::Slot()
			//.HAlign(HAlign_Left)
			//.VAlign(VAlign_Center)
			//.FillWidth(.05F)
			[
				SAlembicImporterWidgetTab::ConstructFocusButton(AssetDataToDisplay)
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

TSharedRef<SButton> SAlembicImporterWidgetTab::ConstructFocusButton(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> FocusButton = SNew(SButton)
		.Text(FText::FromString(TEXT("Focus")))
		.HAlign(HAlign_Right)
		.DesiredSizeScale(FVector2D(.1,.1))
		.OnClicked(this,&SAlembicImporterWidgetTab::OnDeleteButtonClicked,AssetDataToDisplay)
		;

	return FocusButton;
}

TSharedRef<STextBlock> SAlembicImporterWidgetTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White)
		;

	return TextBlock;
}

void SAlembicImporterWidgetTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	DebugHeader::Print(AssetData->AssetName.ToString() + StaticEnum<ECheckBoxState>()->GetNameStringByValue((int64)NewState ), FColor::Green);

}

FReply SAlembicImporterWidgetTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{	
	FCustomPluginModule& CustomPluginModule =
	FModuleManager::LoadModuleChecked<FCustomPluginModule>(TEXT("CustomPlugin"));

	const bool bAssetDeleted = CustomPluginModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		// Updating the list source items
		if (StoredAssetData.Contains(ClickedAssetData))
		{
			StoredAssetData.Remove(ClickedAssetData);
		}

		// Refresh the list
		RefreshAssetListView();

	}

	DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT("is Clicked"), FColor::Green);
	return FReply::Handled();
}
