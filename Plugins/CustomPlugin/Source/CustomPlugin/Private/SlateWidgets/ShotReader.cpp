// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomPlugin.h"

#include "SlateWidgets/ShotReader.h"
#include "DebugHeader.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
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

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "EditorReimportHandler.h"


#pragma region ConstructWidget
void SShotReaderWidgetTab::Construct(const FArguments& InArgs)
{
	// Can Use Keyboard for controlling
	bCanSupportFocus = true;

	ShotDataList = InArgs._ShotDataList;
	ShotDataListBase = InArgs._ShotDataList;
	namingTypes = InArgs._namingTypes;

	SelectedSequenceName = MakeShared<FString>("--- All ---");
	SelectedFilterType = MakeShared<FString>("--- All ---");

	ListComboBoxTypeFilter.Empty();
	ListComboBoxTypeFilter.Add(MakeShared<FString>("--- All ---"));
	for (TPair<FString, FString>& Pair : namingTypes)
	{
		TSharedPtr<FString> PtrValue = MakeShared<FString>(Pair.Value);
		ListComboBoxTypeFilter.Add(PtrValue);
	}
	

	// Sequence Name List
	SequenceNameList.Empty();
	SequenceNameList.Add(MakeShared<FString>("--- All ---"));
	TSet<FString> SeenNames;

	for (TSharedPtr<FShotData> ShotData : ShotDataListBase)
	{
		FString ShotName = ShotData->ShotName;

		if (SeenNames.Contains(ShotName)) continue;
			
		SeenNames.Add(ShotName);

		TSharedPtr<FString> PtrShotName = MakeShared<FString>(ShotName);
		SequenceNameList.Add(PtrShotName);

	}

	// Define Font
	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 20;

	DebugHeader::Print("Construct the Shot Reader");

	// Add Main Widget to the TabWidget
	ChildSlot
		[
			// Create Vertical Box
			SNew(SVerticalBox)
				
				// Top Bar Name
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
						// Shot Filter
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)

						[
							SNew(STextBlock)
								.Text(FText::FromString("Shot Name"))
								.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
								.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))
						]

						
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)

						[
							SNew(SBox)
								.WidthOverride(200)
								[
									SNew(SComboBox<TSharedPtr<FString>>)
										.OnGenerateWidget(this, &SShotReaderWidgetTab::OnGenerateComboSequenceName)
										.OptionsSource(&SequenceNameList)
										.OnSelectionChanged(this, &SShotReaderWidgetTab::OnComboSequenceSelectionChanged)

										[
											SNew(STextBlock)
												.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
												.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))
												.Text_Lambda([this]() {
												return SelectedSequenceName.IsValid() ? FText::FromString(*SelectedSequenceName) : FText::FromString("--- All ---");
													})
										]
								]

						]

						// Type Filter
						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString("File Type"))
									.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
									.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))
							]

						+ SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
									.WidthOverride(200.f)[
										SNew(SComboBox<TSharedPtr<FString>>)
											.OnGenerateWidget(this, &SShotReaderWidgetTab::OnGenerateComboFilterTypeWidget)
											.OptionsSource(&ListComboBoxTypeFilter)
											.OnSelectionChanged(this, &SShotReaderWidgetTab::OnComboSelectionChanged)
									
											[
												SNew(STextBlock)
													.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
													.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))
													.Text_Lambda([this]() {
													return SelectedFilterType.IsValid() ? FText::FromString(*SelectedFilterType) : FText::FromString("--- All ---");
														})
											]
									]
							]

						// Reload Button
						+ SHorizontalBox::Slot()
							.FillWidth(2.0f)
							.HAlign(HAlign_Right)
							[
								SNew(SButton)
									.OnClicked(this, &SShotReaderWidgetTab::OnReloadButtonClicked)

								[
									SNew(STextBlock)
										.Text(FText::FromString("Reload"))
										.Font(FCoreStyle::Get().GetFontStyle("NormalFont"))
										.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
										.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))

								]

							]
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

		+ SHeaderRow::Column(FName("SequenceName"))
		.DefaultLabel(FText::FromString("SQ"))
		.FillWidth(.2f)

		+ SHeaderRow::Column(FName("ShotName"))
		.DefaultLabel(FText::FromString("Shot"))
		.FillWidth(.2f)

		+ SHeaderRow::Column(FName("Version"))
		.DefaultLabel(FText::FromString("Version"))
		.FillWidth(.2f)

		+ SHeaderRow::Column(FName("AssetName"))
		.DefaultLabel(FText::FromString("Asset Name"))
		.FillWidth(.4f)

		+ SHeaderRow::Column(FName("ImportPath"))
		.DefaultLabel(FText::FromString("Current Asset Import File Path"))

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

#pragma endregion

#pragma region BindingWidgetFunction
TSharedRef<ITableRow> SShotReaderWidgetTab::OnGeneratedRowAssetList(TSharedPtr<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<ITableRow> GeneratedRow = SNew(STableRow<TSharedRef<FShotData>>, OwnerTable)
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(ShotDataStruct->IsAssetExists ? FLinearColor(0.0f,1.0f,0.0f,0.2f) : FLinearColor(0.1f, 0.0f, 0.0f, 0.2f))
				[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.FillWidth(.2f)

					[
						SNew(STextBlock).Text(FText::FromString(ShotDataStruct->ShotMainName))
					]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(.2f)

				[
					SNew(STextBlock).Text(FText::FromString(ShotDataStruct->ShotName))
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(.2f)

				[
					SNew(STextBlock).Text(FText::FromString(ShotDataStruct->PolishStateText))

				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.FillWidth(.4f)

				[
					SNew(STextBlock).Text(FText::FromString(ShotDataStruct->AssetName))

				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(FText::FromString(ShotDataStruct->CurrentImportPathShorten))

				]
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
			FText::FromString("Reimport / Update File"),
			FText::FromString("Reimport file to lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this,&SShotReaderWidgetTab::ReimportSelectedItem))
		);


	}

	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SectionExplore", FText::FromString("Explore"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Show in File Explorer"),
		FText::FromString("Import file lastest version"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BrowseFileLocation))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Show in Content Browser"),
		FText::FromString("Reimport file to lastest version"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BrowseAssetLocation))
	);


	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SectionOperation", FText::FromString("Operations"));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Build Sequencer"),
			FText::FromString("Build Sequencer by including character and camera"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SShotReaderWidgetTab::BuildSequencerToSelectedShot))
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SShotReaderWidgetTab::OnGenerateComboFilterTypeWidget(TSharedPtr<FString> InItem)
{
	UE_LOG(LogTemp, Log, TEXT("Genreating Combo bOX"));
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.Margin(FMargin(4.0f, 4.0f));
}

TSharedRef<SWidget> SShotReaderWidgetTab::OnGenerateComboSequenceName(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.Margin(FMargin(4.0f, 4.0f));
}



void SShotReaderWidgetTab::OnComboSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		SelectedFilterType = NewSelection;
		SShotReaderWidgetTab::UpdateShotListFilterWidget();

	}

}

void SShotReaderWidgetTab::OnComboSequenceSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		SelectedSequenceName = NewSelection;
		SShotReaderWidgetTab::UpdateShotListFilterWidget();

	}

}

FReply SShotReaderWidgetTab::OnReloadButtonClicked()
{
	ReloadAll();
	UpdateShotListFilter();
	UE_LOG(LogTemp, Log, TEXT("## Reloaded, Debugging naming Types ##"));

	for (const TPair<FString, FString>& Pair : namingTypes)
	{
		FString CurrentKey = Pair.Key;
		FString CurrentValue = Pair.Value;
		UE_LOG(LogTemp, Log, TEXT("Key: %s, Value: %s"), *CurrentKey, *CurrentValue);
	}


	return FReply::Handled();

}

#pragma endregion

#pragma region RightClickMenu
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
			FString ContentPath = item->ContentAssetFilePath;
			FString NewSourcePath = item->LastestFilePath;
			UObject* Asset = LoadObject<UObject>(nullptr, ContentPath);

			FReimportManager::Instance()->ReimportAsync(Asset, false, true, NewSourcePath);

			ReloadAll();
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

		ReloadAll();

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
	TArray<TSharedPtr<FShotData>> SelectedItems;
	 
	UE_LOG(LogTemp, Log, TEXT("# BrowseAssetLocation"));


	if (!ConstructedAssetListView.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("# ConstructedAssetListView is Invalid"));
		return;
	}

	ConstructedAssetListView->GetSelectedItems(SelectedItems);

	if (SelectedItems.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("# Not Found Any Selected Items"));
		return;
	}
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	
	TArray<FAssetData> AssetDataList;
	FString TargetPath = SelectedItems[0]->ContentAssetFilePath;

	if (!UEditorAssetLibrary::DoesAssetExist(TargetPath))
	{
		UE_LOG(LogTemp, Log, TEXT("# Asset Path Not Exist %s"), *TargetPath);
		return;
	}


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FSoftObjectPath AssetObjectPath(TargetPath);
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(TargetPath);
	AssetDataList.Add(AssetData);

	ContentBrowserModule.Get().SyncBrowserToAssets(AssetDataList);

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

#pragma region Misc
void SShotReaderWidgetTab::ReloadAll()
{
	FCustomPluginModule& CustomModule = FModuleManager::GetModuleChecked<FCustomPluginModule>("CustomPlugin");

	// Update Array Data
	//ShotDataList = CustomModule.GetShotData();
	ShotDataListBase = CustomModule.GetShotData();

	// Refresh the list view
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

void SShotReaderWidgetTab::UpdateShotListFilter()
{

	if (!SelectedFilterType.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("## Updating Shot Data List Filter Failed , SelectedFilterType Pointer still valid ##"));

		return;

	}

	if (!SelectedSequenceName.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("## Updating Shot Data List Filter Failed , SelectedSequenceName Pointer still valid ##"));

		return;

	}

	UE_LOG(LogTemp, Log, TEXT("## Updating Shot Data List Filter ##"));
	UE_LOG(LogTemp, Log, TEXT("- Selected Filter Shot Name : %s"), *(*SelectedFilterType));
	UE_LOG(LogTemp, Log, TEXT("- Selected Filter Type : %s"), *(*SelectedSequenceName));

	// If Shot List Filter Not Select Anythings
	//if (!ListComboBoxTypeFilter.Contains(SelectedFilterType))
	//{
	//	UE_LOG(LogTemp, Log, TEXT("- Selected Filter Types is invalid"));
	//	return;
	//}

	//if (!SequenceNameList.Contains(SelectedSequenceName))
	//{
	//	UE_LOG(LogTemp, Log, TEXT("- Selected Shot Name is invalid"));
	//	return;
	//}



	// If Shot LIst Filter Select Somethings
	TArray<TSharedPtr<FShotData>> ResultShotNameList;
	TArray<TSharedPtr<FShotData>> ResultFileTypeList;



	// Filter Naming Convention
	TArray<TSharedPtr<FShotData>> ShotDataListBaseDefault = ShotDataListBase;

	if (*SelectedFilterType == "--- All ---")
	{
		ResultFileTypeList = ShotDataListBaseDefault;
	}
	else
	{
		// Get Naming Convention
		FString NamingConvention;

		for (const TPair<FString, FString> Pair : namingTypes)
		{
			if (Pair.Value == *SelectedFilterType)
			{
				NamingConvention = Pair.Key;
				UE_LOG(LogTemp, Log, TEXT("- Naming Convention is : %s"), *(Pair.Key));

			}
		}

		if (NamingConvention.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("Naming Convention Not Found"));
			return;
		}

		for (TSharedPtr<FShotData> ShotData : ShotDataListBaseDefault)
		{
			const FString FileName = FPaths::GetCleanFilename(ShotData->LastestFilePath);


			if (FileName.MatchesWildcard(NamingConvention))
			{
				ResultFileTypeList.Add(ShotData);
				UE_LOG(LogTemp, Log, TEXT("- Matching : %s"), *FileName);

			}
		}
	}

	// Filter Shot Name
	const FString ShotName = *SelectedSequenceName;

	if (ShotName == "--- All ---")
	{
		ResultShotNameList = ResultFileTypeList;

	}
	else
	{
		for (TSharedPtr<FShotData> ShotData : ResultFileTypeList)
		{
			if (ShotData->ShotName == ShotName)
			{
				ResultShotNameList.Add(ShotData);
			}
		}
	}

	// Update New Shot Data to List
	ShotDataList = ResultShotNameList;

}

void SShotReaderWidgetTab::UpdateShotListFilterWidget()
{
	UpdateShotListFilter();
	ReloadAll();
}

#pragma endregion