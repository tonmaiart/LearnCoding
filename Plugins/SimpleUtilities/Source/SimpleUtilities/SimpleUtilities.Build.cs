// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleUtilities : ModuleRules
{
	public SimpleUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


        PrivateIncludePaths.AddRange(
            new string[] {
                System.IO.Path.GetFullPath(Target.RelativeEnginePath)+"Source/Editor/Blutility/Private",
                System.IO.Path.GetFullPath(Target.RelativeEnginePath)+"Plugins/MovieScene/LevelSequenceEditor/Source/LevelSequenceEditor/Private/Factories"

            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {

                "ContentBrowser",
                "AssetRegistry",
                "Core",
                "Blutility",
                "EditorScriptingUtilities",
                "UnrealEd",

                "GeometryCache",
                "AlembicImporter",
                "AlembicLibrary",

                "LevelSequence",

                "MovieScene",
                "MovieSceneTracks",
                "MovieSceneTools",

                "CinematicCamera",
                "LevelSequenceEditor",

				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",

               "GeometryCache",
                "AlembicImporter",
                "AlembicLibrary",

                "AssetTools",
                "AssetRegistry",
                //"LevelSeqeuence",
                "LevelSequenceEditor",
                //"MovieScene"

                                //"MovieScene",
                //"MovieSceneTracks",
                //"MovieSceneTools",
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
