// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrowdMovement_JB_GB : ModuleRules
{
	public CrowdMovement_JB_GB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
<<<<<<< HEAD
		PublicDependencyModuleNames.AddRange(new string[] { 
			//Base
			"Core",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",

			//Mass AI
			"MassEntity",
			"MassCommon",
			"MassSpawner",
            "MassAIBehavior",
			
			//MassGameplay
			"MassGameplayDebug",
			"MassActors",
			"MassMovement",
			"MassNavigation",
			"MassSignals",
			"MassSmartObjects",
			"NavigationSystem",

			// MassCrowd
			"MassCrowd",
			"ZoneGraph",
			"ZoneGraphAnnotations",
			
			//Navigation
			"Navmesh", 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
