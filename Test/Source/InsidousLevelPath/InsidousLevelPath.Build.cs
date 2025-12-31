// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InsidousLevelPath : ModuleRules
{
	public InsidousLevelPath(ReadOnlyTargetRules Target) : base(Target)
	{
        // bUsePrecompiled = true;
        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "Engine",
                "AndroidRuntimeSettings",
                "OpenSSLLib",
                "LevelSequence",
				"MovieScene",
				"MovieSceneTracks",
				//"MovieSceneTools",
				//"MovieSceneCapture",
            }
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

        PublicDefinitions.Add("UE_WITH_DTLS=1");
        PublicDependencyModuleNames.Add("SSL");
    }
}