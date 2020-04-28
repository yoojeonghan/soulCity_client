// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

/*------------------------------------------------------------------------------*/
#include "MyGameInstance.generated.h"

UCLASS()
class KELSAIK_20180623_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyGameInstance(const FObjectInitializer & ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolusHUDCrosshair)
	int32 IsLogin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolusHUDCrosshair)
	int32 IsTCPLogin;
};
