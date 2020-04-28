// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FileHelper.h"
#include "Paths.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "kelsaik_20180623GameMode.h"
/*------------------------------------------------------------------------------*/
#include "SessionActor.generated.h"

UCLASS(Blueprintable, hideCategories = (Rendering, Replication, Input, Actor, "Actor Tick"))
class KELSAIK_20180623_API ASessionActor : public AActor
{
	GENERATED_BODY()
	
private:
	FHttpModule * Http;
	//FString ApiBaseUrl = "http://localhost:5000/api/";
	FString ApiBaseUrl = "http://52.78.90.90:80/api/";

	FString AuthorizationHeader = TEXT("Authorization");
	void SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> RequestWithRoute(FString Subroute);
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> GetRequest(FString Subroute);
	TSharedRef<IHttpRequest> PostRequest(FString Subroute, FString ContentJsonString);
	void Send(TSharedRef<IHttpRequest>& Request);

	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	template <typename StructType>
	void GetJsonStringFromStruct(StructType FilledStruct, FString& StringOutput);
	template <typename StructType>
	void GetStructFromJsonString(FHttpResponsePtr Response, StructType& StructOutput);

public:	
	// Sets default values for this actor's properties
	ASessionActor();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//void Login(FRequest_Login LoginCredentials);
	//void LoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

/*
USTRUCT()
struct FRequest_Login
{
	GENERATED_BODY()
	UPROPERTY() FString email;
	UPROPERTY() FString password;

	FRequest_Login() {}
};

USTRUCT()
struct FResponse_Login
{
	GENERATED_BODY()
	UPROPERTY() int id;
	UPROPERTY() FString name;
	UPROPERTY() FString hash;

	FResponse_Login() {}
};

*/