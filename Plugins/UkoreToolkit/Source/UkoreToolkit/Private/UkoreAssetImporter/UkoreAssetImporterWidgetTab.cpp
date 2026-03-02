// Fill out your copyright notice in the Description page of Project Settings.

#include "UkoreAssetImporter/UkoreAssetImporterWidgetTab.h"
#include "UkoreAssetImporter/UkoreAssetImporter.h"
#include "UkoreToolkitSettings.h"

#include "UkoreToolkit.h"
#include "UkoreCore/UkoreUtility.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformProcess.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
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



void SUkoreAssetImporterWidgetTab::Construct(const FArguments& InArgs)
{
	// Can Use Keyboard for controlling
	bCanSupportFocus = true;

	// Fetching
	FetchConfiguration();
	FetchExistShotDataList();
	FetchDataForListComboBox();

	// Set Default Selected
	SelectedSequenceName = MakeShared<FString>("--- All ---");
	SelectedFilterType = MakeShared<FString>("--- All ---");

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
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
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
										.OnGenerateWidget(this, &SUkoreAssetImporterWidgetTab::OnGenerateComboSequenceName)
										.OptionsSource(&SequenceNameList)
										.OnSelectionChanged(this, &SUkoreAssetImporterWidgetTab::OnComboSequenceSelectionChanged)

										[
											SNew(STextBlock)
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
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
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
								.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))
						]

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
								.WidthOverride(200.f)[
									SNew(SComboBox<TSharedPtr<FString>>)
										.OnGenerateWidget(this, &SUkoreAssetImporterWidgetTab::OnGenerateComboFilterTypeWidget)
										.OptionsSource(&ListComboBoxTypeFilter)
										.OnSelectionChanged(this, &SUkoreAssetImporterWidgetTab::OnComboSelectionChanged)

										[
											SNew(STextBlock)
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
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
								.OnClicked(this, &SUkoreAssetImporterWidgetTab::OnReloadButtonClicked)

								[
									SNew(STextBlock)
										.Text(FText::FromString("Reload"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										.Margin(FMargin(5.0f, 2.0f, 5.0f, 2.0f))

								]

						]
				]

			// SListView
			+ SVerticalBox::Slot()
				[
					SUkoreAssetImporterWidgetTab::ConstructAssetListView()
				]
		];

		ReloadAll();
}


TSharedRef<SListView<TSharedPtr<FShotData>>> SUkoreAssetImporterWidgetTab::ConstructAssetListView()
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
		.OnGenerateRow(this, &SUkoreAssetImporterWidgetTab::OnGeneratedRowAssetList)
		.OnContextMenuOpening(this, &SUkoreAssetImporterWidgetTab::OnGeneratedContextMenu)
		.HeaderRow(HeaderRow)
		;


	return ConstructedAssetListView.ToSharedRef();

}

#pragma endregion

#pragma region BindingWidgetFunction
TSharedRef<ITableRow> SUkoreAssetImporterWidgetTab::OnGeneratedRowAssetList(TSharedPtr<FShotData> ShotDataStruct, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<ITableRow> GeneratedRow = SNew(STableRow<TSharedRef<FShotData>>, OwnerTable)
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				// Use _Lambda so the widget knows to execute your logic
				.BorderBackgroundColor_Lambda([ShotDataStruct]() -> FSlateColor
					{
						// Use the AssetStatus variable from your struct
						switch (ShotDataStruct->AssetStatus)
						{
						case EAssetStatus::Unloaded:
							return FLinearColor(0.0f, 0.0f, .0f, 0.f);
						case EAssetStatus::Loaded:
							return FLinearColor(1.0f, 0.5f, 0.0f, 0.3f); // Yellow
						case EAssetStatus::UpToDate:
							return FLinearColor(0.23f, 0.3f, 0.0f, .5f); // Green
						default:
							return FLinearColor(1.0f, 1.0f, 1.0f, 0.3f); // Default
						}
					})

				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
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

TSharedPtr<SWidget> SUkoreAssetImporterWidgetTab::OnGeneratedContextMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("SectionCommon", FText::FromString("Common"));

	{

		MenuBuilder.AddMenuEntry(
			FText::FromString("Import File"),
			FText::FromString("Import file lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SUkoreAssetImporterWidgetTab::OnImportSelectedItemClicked))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Reimport / Update File"),
			FText::FromString("Reimport file to lastest version"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SUkoreAssetImporterWidgetTab::OnReimportSelectedItemClicked))
		);


	}

	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SectionExplore", FText::FromString("Explore"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Show in File Explorer"),
		FText::FromString("Import file lastest version"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUkoreAssetImporterWidgetTab::OnBrowseFileLocationClicked))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Show in Content Browser"),
		FText::FromString("Reimport file to lastest version"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SUkoreAssetImporterWidgetTab::OnBrowseAssetLocation))
	);


	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SectionOperation", FText::FromString("Operations"));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Build Sequencer"),
			FText::FromString("Build Sequencer by including character and camera"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SUkoreAssetImporterWidgetTab::OnBuildSequencerToSelectedShot))
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SUkoreAssetImporterWidgetTab::OnGenerateComboFilterTypeWidget(TSharedPtr<FString> InItem)
{
	UE_LOG(LogTemp, Log, TEXT("Genreating Combo bOX"));
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.Margin(FMargin(4.0f, 4.0f));
}

TSharedRef<SWidget> SUkoreAssetImporterWidgetTab::OnGenerateComboSequenceName(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.Margin(FMargin(4.0f, 4.0f));
}



void SUkoreAssetImporterWidgetTab::OnComboSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		SelectedFilterType = NewSelection;
		SUkoreAssetImporterWidgetTab::UpdateShotListFilterWidget();

	}

}

void SUkoreAssetImporterWidgetTab::OnComboSequenceSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		SelectedSequenceName = NewSelection;
		SUkoreAssetImporterWidgetTab::UpdateShotListFilterWidget();

	}

}

FReply SUkoreAssetImporterWidgetTab::OnReloadButtonClicked()
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

TArray<TSharedPtr<FShotData>> SUkoreAssetImporterWidgetTab::GetSelectedItemListView()
{
	if (!ConstructedAssetListView.IsValid()) return TArray<TSharedPtr<FShotData>>();

	TArray<TSharedPtr<FShotData>> SelectedItems;
	ConstructedAssetListView->GetSelectedItems(SelectedItems);
	return SelectedItems;
}
void SUkoreAssetImporterWidgetTab::OnImportSelectedItemClicked()
{
	// Get Selected Items
	TArray<TSharedPtr<FShotData>> SelectedItems = SUkoreAssetImporterWidgetTab::GetSelectedItemListView();

	for (TSharedPtr<FShotData> item : SelectedItems)
	{
		FString& ImportPath = item->LastestFilePath;
		FString& DestinationDirPath = item->ContentAssetDirPath;

		UE_LOG(LogTemp, Log, TEXT("# Checking File Path : "), *ImportPath);

		// Check Asset Existising in Content Browser
		if (item->AssetStatus == EAssetStatus::Loaded || item->AssetStatus == EAssetStatus::UpToDate)
		{
			UE_LOG(LogTemp, Log, TEXT("# Importing Asset : "), *ImportPath);

		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("# Skip Because this asset already imported : "), *ImportPath);
			continue;

		}

		// Create Directory if not have
		if (!UEditorAssetLibrary::DoesDirectoryExist(DestinationDirPath))
		{
			UEditorAssetLibrary::MakeDirectory(DestinationDirPath);
		}

		// Import Alembic File to Directory
		Utility::ImportAlembicFile(ImportPath, DestinationDirPath);

		DebugHeader::Print("Import Asset Complete ! : " + ImportPath);
	}

	ReloadAll();
}

void SUkoreAssetImporterWidgetTab::OnReimportSelectedItemClicked()
{
	DebugHeader::Print(" # Reimport Clicked");

	// Get Selected Items
	TArray<TSharedPtr<FShotData>> SelectedItems = SUkoreAssetImporterWidgetTab::GetSelectedItemListView();

	for (TSharedPtr<FShotData> item : SelectedItems)
	{
		FString ContentPath = item->ContentAssetFilePath;
		FString NewSourcePath = item->LastestFilePath;
		UObject* Asset = LoadObject<UObject>(nullptr, ContentPath);

		FReimportManager::Instance()->ReimportAsync(Asset, false, true, NewSourcePath);

	}
	ReloadAll();
}

void SUkoreAssetImporterWidgetTab::OnBrowseFileLocationClicked()
{
	TArray<TSharedPtr<FShotData>> SelectedItems = GetSelectedItemListView();

	if (SelectedItems.IsEmpty())
	{
		return;
	}

	FString DirBrowse = FPaths::GetPath(SelectedItems[0]->LastestFilePath);
	FPlatformProcess::ExploreFolder(*(SelectedItems[0]->LastestFilePath));
}

void SUkoreAssetImporterWidgetTab::OnBrowseAssetLocation()
{
	TArray<TSharedPtr<FShotData>> SelectedItems = GetSelectedItemListView();
	FString TargetPath = SelectedItems[0]->ContentAssetFilePath;

	UE_LOG(LogTemp, Log, TEXT("# BrowseAssetLocation"));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");

	TArray<FAssetData> AssetDataList;

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

void SUkoreAssetImporterWidgetTab::OnBuildSequencerToSelectedShot()
{
	FString CameraPath = TEXT("C:\\publish\\AOA\\AOA101\\Layout\\v006\\camera_maya.fbx");
	UObject* NewSequencer = Utility::BuildSequencer("/Game/Sequences/AOA/AOA101/SEQ_AOA101_Test",
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


#pragma region Widget Function
void SUkoreAssetImporterWidgetTab::ReloadAll()
{
	// Update Array Data
	SUkoreAssetImporterWidgetTab::FetchExistShotDataList();
	SUkoreAssetImporterWidgetTab::UpdateShotListFilter();

	// Refresh the list view
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

void SUkoreAssetImporterWidgetTab::UpdateShotListFilter()
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


	// If Shot LIst Filter Select Somethings
	TArray<TSharedPtr<FShotData>> ResultShotNameList;
	TArray<TSharedPtr<FShotData>> ResultFileTypeList;

	// Filter Naming Convention
	TArray<TSharedPtr<FShotData>> &ShotDataListBaseDefault = ShotDataListBase;

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

void SUkoreAssetImporterWidgetTab::UpdateShotListFilterWidget()
{
	UpdateShotListFilter();
	ReloadAll();
}

void SUkoreAssetImporterWidgetTab::FetchDataForListComboBox()
{
	ListComboBoxTypeFilter.Empty();
	for (TPair<FString, FString>& Pair : namingTypes)
	{
		TSharedPtr<FString> PtrValue = MakeShared<FString>(Pair.Value);
		ListComboBoxTypeFilter.Add(PtrValue);
	}

	ListComboBoxTypeFilter.Sort();
	ListComboBoxTypeFilter.Insert(MakeShared<FString>("--- All ---"), 0);

	// Sequence Name List
	SequenceNameList.Empty();
	TSet<FString> SeenNames;

	for (TSharedPtr<FShotData> ShotData : ShotDataListBase)
	{
		FString ShotName = ShotData->ShotName;

		if (SeenNames.Contains(ShotName)) continue;

		SeenNames.Add(ShotName);

		TSharedPtr<FString> PtrShotName = MakeShared<FString>(ShotName);
		SequenceNameList.Add(PtrShotName);

	}

	//SequenceNameList.Sort();
	SequenceNameList.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
		{
			return *A < *B;  // dereference to compare actual strings
		});
	SequenceNameList.Insert(MakeShared<FString>("--- All ---"), 0);


}

void SUkoreAssetImporterWidgetTab::FetchConfiguration()
{
	UE_LOG(LogTemp, Log, TEXT("## Fetching Configuration from Settings ## "));

	const UUkoreToolkitSettings* Settings = GetDefault<UUkoreToolkitSettings>();

	if (Settings)
	{
		UE_LOG(LogTemp, Log, TEXT("- Settings Founded"));

		ShotRootPaths = Settings->ShotRootPaths;
		SubFolder = Settings->SubFolder;
		ContentShotRootPath = Settings->ContentShotRootPath;
		namingTypes = Settings->namingTypes;

		for (const TPair<FString, FString>& Pair : namingTypes)
		{
			// Access the Key and Value using Pair.Key and Pair.Value
			UE_LOG(LogTemp, Log, TEXT("Key: %s | Value: %s"), *Pair.Key, *Pair.Value);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("- Settings Not Found!"));

	}


}
void SUkoreAssetImporterWidgetTab::FetchExistShotDataList()
{
	ShotDataListBase = AssetImporterUtility::GetShotDataArray
	(
		ShotRootPaths,
		SubFolder,
		namingTypes,
		ContentShotRootPath
	);
}

#pragma endregion