// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/ShotReader.h"
#include "DebugHeader.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
//#include "AbcImportFactory.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SimpleUtilities.h"
#include "EditorAssetLibrary.h"

#include "Modules/ModuleManager.h"
#include "IAssetTools.h"

#include "GeometryCache.h"
#include "AlembicImportFactory.h"
#include "AbcImportSettings.h"
#include "CoreMinimal.h"


#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "MovieScene.h"
#include "MovieSceneToolHelpers.h"
#include "MovieScenePossessable.h"
#include "MovieSceneToolsUserSettings.h"

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


TSharedRef<SListView<TSharedPtr<FShotData>>> SShotReaderWidgetTab::ConstructAssetListView()
{

	// Create Header Row
	TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow)

		+ SHeaderRow::Column(FName("ShotName"))
		.DefaultLabel(FText::FromString("Shot Name"))
		.FillWidth(.2f)

		+ SHeaderRow::Column(FName("Last"))
		.DefaultLabel(FText::FromString("Lastest Version"))
		.FillWidth(.08f)

		+ SHeaderRow::Column(FName("Current"))
		.DefaultLabel(FText::FromString("Current Version"))
		.FillWidth(.08f)

		+ SHeaderRow::Column(FName("AssetName"))
		.DefaultLabel(FText::FromString("Asset Name"))
		.FillWidth(.5f)

		+ SHeaderRow::Column(FName("ImportPath"))
		.DefaultLabel(FText::FromString("Import Path"))

		;

	DebugHeader::Print("Construcing Asset List View");

	ConstructedAssetListView = SNew(SListView<TSharedPtr<FShotData>>)
		.ListItemsSource(&ShotDataList)
		.OnGenerateRow(this, &SShotReaderWidgetTab::OnGeneratedRowAssetList)
		.OnContextMenuOpening(this, &SShotReaderWidgetTab::OnGeneratedContextMenu)
		.HeaderRow(HeaderRow)
		;
	

	return ConstructedAssetListView.ToSharedRef();
	
}

//TSharedRef<SWidget> SShotReaderWidgetTab::GenerateWidgetForColumn(const FName& ColumnName)
//{
//	//return SNew(SWidget,nullptr);
//	return;
//}

TSharedRef<ITableRow> SShotReaderWidgetTab::OnGeneratedRowAssetList(TSharedPtr<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<ITableRow> GeneratedRow = SNew(STableRow<TSharedRef<FShotData>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(.2f)

				[
					SNew(STextBlock).Text(FText::FromString(ShotDataStruct->ShotMainName))
				]

			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.FillWidth(.08f)

			[
				SNew(STextBlock).Text(FText::FromString(FString::FromInt(ShotDataStruct->LastestVersion)))

			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.FillWidth(.08f)

			[
				SNew(STextBlock).Text(FText::FromString(FString::FromInt(ShotDataStruct->CurrentVersion)))

			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.FillWidth(.5f)

			[
				SNew(STextBlock).Text(FText::FromString(ShotDataStruct->AssetName))

			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock).Text(FText::FromString(ShotDataStruct->LastestFilePath))

			]
		];
	
	return GeneratedRow;
}

TSharedPtr<SWidget> SShotReaderWidgetTab::OnGeneratedContextMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("SectionCommon", FText::FromString("Common"));

	{

		MenuBuilder.AddMenuEntry(
			FText::FromString("Import File"),
			FText::FromString("Import file lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::ImportSelectedItem))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Reimport File (Update)"),
			FText::FromString("Reimport file to lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this,&SShotReaderWidgetTab::ReimportSelectedItem))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Browse File in File Explorer"),
			FText::FromString("Import file lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BrowseFileLocation))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Browse Asset File in Content Browser"),
			FText::FromString("Reimport file to lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BrowseAssetLocation))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Build Sequencer"),
			FText::FromString("Build Sequencer by include character , camera"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BuildSequencerToSelectedShot))
		);

	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SShotReaderWidgetTab::ReimportSelectedItem()
{
	if (!ConstructedAssetListView.IsValid()) return;


	DebugHeader::Print("Reimport Clicked");
	TArray<TSharedPtr<FShotData>> SelectedItems;
	ConstructedAssetListView->GetSelectedItems(SelectedItems);

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->GetSelectedItems(SelectedItems);

		for (TSharedPtr<FShotData> item : SelectedItems)
		{

			FString DirBrowse = FPaths::GetPath(item->LastestFilePath);
			FPlatformProcess::ExploreFolder(*(item->LastestFilePath));
			//Utility::CheckAlembicImportPath();
			DebugHeader::Print("Browse File Location" + DirBrowse);



		}

	}
}

void SShotReaderWidgetTab::ImportSelectedItem()
{
	UE_LOG(LogTemp, Log, TEXT("## Import Selected Item Start ##"));

	if (!ConstructedAssetListView.IsValid()) return;

	UE_LOG(LogTemp, Log, TEXT("# Constructed Asset List View is Valid , Ready to Continue..."));

	// Get Selected Items
	TArray<TSharedPtr<FShotData>> SelectedItems;
	ConstructedAssetListView->GetSelectedItems(SelectedItems);

	for (TSharedPtr<FShotData> item : SelectedItems)
	{

		FString ImportPath = item->LastestFilePath;
		
		FString ShotName = item->ShotName;
		FString ShotMainName = item->ShotMainName;

		FString DestinationDirPath = item->ContentAssetDirPath;

		UE_LOG(LogTemp, Log, TEXT("# Checking File Path : "),*ImportPath);

		// Skip if Asset is Already Exist
		if (item->IsAssetExists) continue;

		UE_LOG(LogTemp, Log, TEXT("# Starting Import..."), *ImportPath);

		// Create Directory if not have
		if (!UEditorAssetLibrary::DoesDirectoryExist(DestinationDirPath))
		{
			UEditorAssetLibrary::MakeDirectory(DestinationDirPath);
		}

		// Import Alembic File to Directory
		Utility::ImportAlembicFile(ImportPath, DestinationDirPath);

		item->IsAssetExists = true;

		DebugHeader::Print("Import File From : " + ImportPath);
		DebugHeader::Print("Import File To : " + DestinationDirPath);


	}


}

void SShotReaderWidgetTab::BrowseFileLocation()
{
	TArray<TSharedPtr<FShotData>> SelectedItems;

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->GetSelectedItems(SelectedItems);

		for (TSharedPtr<FShotData> item : SelectedItems)
		{
			FString DirBrowse = FPaths::GetPath(item->LastestFilePath);
			FPlatformProcess::ExploreFolder(*(item->LastestFilePath));
			DebugHeader::Print("Browse File Location" + DirBrowse);



		}

	}

}

void SShotReaderWidgetTab::BrowseAssetLocation()
{
	DebugHeader::Print("Browse Asset Location Clicked");
}

void SShotReaderWidgetTab::BuildSequencerToSelectedShot()
{
	FString CameraPath = TEXT("C:\\publish\\AOA\\AOA101\\Layout\\v006\\camera_maya.fbx");
	UObject* NewSequencer =  Utility::BuildSequencer("/Game/Sequences/AOA/AOA101/SEQ_AOA101_Test",
			TArray<FString>(),
			CameraPath);
	
	if (IsValid(NewSequencer))
	{
		UE_LOG(LogTemp, Log, TEXT("# This Sequencer is valid %s"), *NewSequencer->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("# Failed to build Sequencer"));

	}
}

#pragma endregion