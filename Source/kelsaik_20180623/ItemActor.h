// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "EngineUtils.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Json.h"
#include "JsonUtilities.h"
/*-----------------------------------------------*/
#include "ItemActor.generated.h"

UCLASS()
class KELSAIK_20180623_API AItemActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AItemActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void VisibleItem();

	void NonVisibleItem();

	UFUNCTION(BlueprintCallable)
	void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	void RequestItemLooting();

	void ResponseItemLooting(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* The actual HTTP call */
	UFUNCTION()
	void MyHttpCall();

	/*Assign this function to call when the GET request processes sucessfully*/
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	//UFUNCTION(BlueprintCallable)
	//void EndOverlap(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	bool IsHidden;

	FString PlayerID;

	UPROPERTY()
	class AUDPActor* UDPActor;

	FHttpModule* Http;
};