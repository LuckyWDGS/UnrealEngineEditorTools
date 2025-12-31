// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EditorTools : ModuleRules
{
	public EditorTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"RenderCore",
				"RHI",
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
				"Projects",
				"MeshDescription",
				"StaticMeshDescription",
				"RawMesh",
				"AssetRegistry",
				"UnrealEd",
				"AssetTools",
				"ToolMenus",
				"Persona",
				"SkeletalMeshEditor",
				"AnimationEditor",
				"UnrealEd",
				"MaterialEditor",
				"LevelEditor",
				"Blutility",
				"UMGEditor",
				"MessageLog",
				"ContentBrowser",
				"Renderer",
				// ... add private dependencies that you statically link with here ...
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
