// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ThirdPersonTest : ModuleRules
{
	public ThirdPersonTest(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
    }
}
