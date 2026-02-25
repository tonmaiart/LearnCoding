// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSimpleUtilitiesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsPathSecure(const FString& AssetPath);
};


namespace Utility
{ 
	struct FVersionResult
	{
		FString Path = "";
		int32 Version = 0;

	};

	static TArray<FString> GetDirectoryContent(FString DirectoryPath, bool GetDir, bool GetFile)
{
	// Prepare the output array
	TArray<FString> FoundList;
	TArray<FString> ResultList;

	// Define the visitor
	struct FLocalVisitor : public IPlatformFile::FDirectoryVisitor
	{
		TArray<FString>& OutArray;
		FLocalVisitor(TArray<FString>& InArray) : OutArray(InArray) {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			OutArray.Add(FilenameOrDirectory);
			return true;
		}
	};

	// Execute Search
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalVisitor Visitor(FoundList);
	PlatformFile.IterateDirectory(*DirectoryPath, Visitor);

	// Filter
	for (const FString item : FoundList)
	{
		if (PlatformFile.DirectoryExists(*item) && GetDir == true)
		{
			ResultList.Add(item);
		}
		else if (PlatformFile.FileExists(*item) && GetFile == true)
		{
			ResultList.Add(item);
		}
	}

	return ResultList;
}
	
	/* Get Lastest Folder of Given Directory Path */
	static FVersionResult GetLastestVersionFolder(const FString ParentPath)
	{
		TArray<FString> AllSubDirs = GetDirectoryContent(ParentPath, true, false);
		TArray<FString> VersionFolder;
		TArray<int32> AllVersion;

		FRegexPattern Pattern(TEXT("^v\\d{3}$"));

		for (const FString& FullPath : AllSubDirs)
		{
			// Get only folder name from path
			FString FolderName = FPaths::GetCleanFilename(FullPath);

			FRegexMatcher Matcher(Pattern, FolderName);

			if (Matcher.FindNext())
			{
				// Get Path
				VersionFolder.Add(FullPath);
				
				// Get Version
				FString VersionString = Matcher.GetCaptureGroup(1);
				AllVersion.Add(FCString::Atoi(*VersionString));
			}
		}
		
		FVersionResult ReturnResult;

		// Return Empty String if nothing found
		if (VersionFolder.Num() == 0)
		{
			return ReturnResult;
		}
		else
		{
			VersionFolder.Sort();
			AllVersion.Sort();

			ReturnResult.Path = VersionFolder.Last();
			ReturnResult.Version = AllVersion.Last();
			return ReturnResult;
		}
	}
}