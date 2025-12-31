// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OpenSSLLib : ModuleRules
{
	public OpenSSLLib(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string OpenSSLPath = Path.Combine(ModuleDirectory, "1.1.1t");

		string PlatformSubdir = Target.Platform.ToString();
		string ConfigFolder = (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT) ? "Debug" : "Release";

		if (Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicSystemIncludePaths.Add(Path.Combine(OpenSSLPath, "include", PlatformSubdir));

			string LibPath = Path.Combine(OpenSSLPath, "lib", PlatformSubdir);

			string LibExt = (Target.Platform == UnrealTargetPlatform.IOS && Target.Architecture == UnrealArch.IOSSimulator) ? ".sim.a" : ".a";
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libssl" + LibExt));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libcrypto" + LibExt));
		}
		else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			PlatformSubdir = "Win64";
			string VSVersion = "VS2015";

			// Add includes
			PublicSystemIncludePaths.Add(Path.Combine(OpenSSLPath, "include", PlatformSubdir, VSVersion));

			// Add Libs
			string LibPath = Path.Combine(OpenSSLPath, "lib", PlatformSubdir, VSVersion, ConfigFolder);

			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libssl.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libcrypto.lib"));
			PublicSystemLibraries.Add("crypt32.lib");

            System.Console.WriteLine("UnrealPlatformGroup.Windows" + Path.Combine(OpenSSLPath, "include", PlatformSubdir, VSVersion));
        }
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			string IncludePath = Path.Combine(OpenSSLPath, "include", "Unix");
			string LibraryPath = Path.Combine(OpenSSLPath, "lib", "Unix", Target.Architecture.LinuxName);

			PublicSystemIncludePaths.Add(IncludePath);
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "libssl.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "libcrypto.a"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string IncludePath = Path.Combine(OpenSSLPath, "include", "Android");
			string LibraryPath = Path.Combine(OpenSSLPath, "lib", "Android");

			string[] Architectures = new string[] {
				"ARM64",
				"x86",
				"x64",
			};

			PublicSystemIncludePaths.Add(IncludePath);

			foreach(var Architecture in Architectures)
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, Architecture, "libcrypto.a"));
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, Architecture, "libssl.a"));
			}
		}
	}
}
