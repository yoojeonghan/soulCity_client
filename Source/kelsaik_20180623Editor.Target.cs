// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class kelsaik_20180623EditorTarget : TargetRules
{
	public kelsaik_20180623EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("kelsaik_20180623");

        this.bEnforceIWYU = true;
        this.bCompileLeanAndMeanUE = true;
        this.bForceEnableRTTI = true;
    }
}
