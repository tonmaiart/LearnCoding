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

	//return SNew(SListView<TSharedPtr<FShotData>>);
	DebugHeader::Print("Construcing Asset List View");

	return SNew(SListView<TSharedRef<FShotData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&ShotDataList)
		.OnGenerateRow(this, &SShotReaderWidgetTab::OnGeneratedRowAssetList);

}

TSharedRef<ITableRow> SShotReaderWidgetTab::OnGeneratedRowAssetList(TSharedRef<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString ShotMainName = ShotDataStruct->ShotMainName;
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
					SNew(STextBlock).Text(FText::FromString(ShotMainName))

				]

			// Recent Version
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