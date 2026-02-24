// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/ShotReader.h"
#include "DebugHeader.h"
//#include "ShotReader.h"

#pragma region ConstructWidget

// Main Construct Function
void SShotReaderWidgetTab::Construct(const FArguments& InArgs)
{
	// Can Use Keyboard for controlling
	bCanSupportFocus = true;
	ShotDataList = InArgs._ShotDataList;

	// Define
	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 20;

	DebugHeader::Print("Construct the Shot Reader");

	// Add Main Widget to the TabWidget
	ChildSlot
		[
			// Create Vertical Box
			SNew(SVerticalBox)
				
				// Title Box Name
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Shot Reader")))
						.Font(TitleTextFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)

				]

				// SListView
				+ SVerticalBox::Slot()
				[
					SShotReaderWidgetTab::ConstructAssetListView()
				]
		];
}


TSharedRef<SListView<TSharedRef<FShotData>>> SShotReaderWidgetTab::ConstructAssetListView()
{

	// Create Header Row
	TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow)

		+ SHeaderRow::Column(FName("ShotName"))
		.DefaultLabel(FText::FromString("Shot Name"))

		+ SHeaderRow::Column(FName("LastestVersion"))
		.DefaultLabel(FText::FromString("Lastest Version"))

		+ SHeaderRow::Column(FName("CurrentVersion"))
		.DefaultLabel(FText::FromString("Current Version"))

		+ SHeaderRow::Column(FName("AssetName"))
		.DefaultLabel(FText::FromString("Asset Name"))

		+ SHeaderRow::Column(FName("ImportPath"))
		.DefaultLabel(FText::FromString("Import Path"))
		;

	//return SNew(SListView<TSharedPtr<FShotData>>);
	DebugHeader::Print("Construcing Asset List View");

	return SNew(SListView<TSharedRef<FShotData>>)
		.ListItemsSource(&ShotDataList)
		.OnGenerateRow(this, &SShotReaderWidgetTab::OnGeneratedRowAssetList)
		.HeaderRow(HeaderRow)
		;

}

TSharedRef<ITableRow> SShotReaderWidgetTab::OnGeneratedRowAssetList(TSharedRef<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString ShotMainName = ShotDataStruct->ShotMainName;
	FString ShotName = ShotDataStruct->ShotName;
	int32 LastestVersion = ShotDataStruct->ShotVersion;

	DebugHeader::Print("Generating Row Asset List" + ShotMainName);

	TSharedRef<ITableRow> GeneratedRow = SNew(STableRow<TSharedRef<FShotData>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			
			// Name
			+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(FText::FromString(ShotMainName))
				]

			// Path
			+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(FText::FromString(FString::FromInt(LastestVersion)))

				]

			// Recent Version
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(FText::FromString(FString::FromInt(LastestVersion)))

				]

			// Last Version
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(FText::FromString(ShotMainName))

				]

			// Last Version
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock).Text(FText::FromString(ShotMainName))

			]
		];
	
	return GeneratedRow;
}

#pragma endregion