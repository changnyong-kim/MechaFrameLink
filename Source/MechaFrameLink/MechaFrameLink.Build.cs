// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MechaFrameLink : ModuleRules
{
    public MechaFrameLink(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",

            "UMG",
            "CommonUI",
            "ModelViewViewModel"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore"
        });
    }
}