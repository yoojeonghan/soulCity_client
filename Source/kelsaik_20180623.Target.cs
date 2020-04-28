// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class kelsaik_20180623Target : TargetRules
{
    public kelsaik_20180623Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        ExtraModuleNames.Add("kelsaik_20180623");

        this.bEnforceIWYU = true;
        this.bCompileLeanAndMeanUE = true;
        this.bForceEnableRTTI = true;
        this.bForceEnableExceptions = true;
        //this.bEnableExceptions = false;
    }
}