// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class kelsaik_20180623 : ModuleRules
{
	public kelsaik_20180623(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HeadMountedDisplay",
            "Http",
            "Json",
            "JsonUtilities",
            "Sockets",
            "Networking",
            "UMG",
            "Slate",
            "SlateCore",
            "CryptoPP"
        });

        PublicDefinitions.Add("BOOST_SYSTEM_NOEXCEPT");
        //UEBuildConfiguration.bForceEnableExceptions = true;
        //bForceEnableExceptions = true;
    }
}
