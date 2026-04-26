// Copyright Tirefly. All Rights Reserved.

using UnrealBuildTool;

public class TireflySquadNavigationTest : ModuleRules
{
	public TireflySquadNavigationTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"TireflySquadNavigation",
				"AIModule",
				"NavigationSystem",
				"GameplayTasks",
			}
		);
	}
}
