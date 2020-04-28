// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "UDPActor.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsHidden = true;

	this->OnActorBeginOverlap.AddDynamic(this, &AItemActor::BeginOverlap);
	Http = &FHttpModule::Get();
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AUDPActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		UDPActor = *ActorItr;
	}

	FString sessiontemp;
	FFileHelper::LoadFileToString(sessiontemp, *(FPaths::ProjectDir() + FString("GameSession")));
	PlayerID = sessiontemp;
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsHidden)
	{
		this->SetActorHiddenInGame(true);
	}
	else
	{
		this->SetActorHiddenInGame(false);

	}
}

void AItemActor::VisibleItem()
{
	IsHidden = false;
}

void AItemActor::NonVisibleItem()
{
	IsHidden = true;
}

void AItemActor::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!IsHidden)
	{	 
		if (GEngine)
		{
			if (OtherActor->ActorHasTag("UserCharacter"))
			{
				UDPActor->SendItemLooting();
				MyHttpCall();
			}
		}
	}

}

/*Http call*/
void AItemActor::MyHttpCall()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, "Request");
	}
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AItemActor::OnResponseReceived);
	//This is the url on which to process the request
	FString RequestURL = "http://52.78.90.90:80/SoulCity_Item.php?";
	RequestURL.Append("PlayerID=");
	RequestURL.Append(PlayerID);
	Request->SetURL(RequestURL);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	//Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("Content-Type", TEXT("text/plain"));
	//Request->Set
	//SetContentAsString
	//Request->SetContentAsString(ContentJsonString);
	Request->ProcessRequest();
}

/*Assigned function on successfull http call*/
void AItemActor::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{

	FString ResponseString = Response->GetContentAsString();
	//Create a pointer to hold the json serialized data
	//TSharedPtr<FJsonObject> JsonObject;

	//Create a reader pointer to read the json data
	//TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	if (GEngine)
	{
		if (bWasSuccessful)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ResponseString);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FALSE"));
		}
	}
}

void AItemActor::RequestItemLooting()
{
	/*
	//FString ContentJsonString;
	TSharedRef<IHttpRequest> Request = PostRequest("user/login", PlayerID);
	Request->OnProcessRequestComplete().BindUObject(this, &AItemActor::ResponseItemLooting);
	Send(Request);
	*/
}

void AItemActor::ResponseItemLooting(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	/*
	if (!ResponseIsValid(Response, bWasSuccessful))
	{
		return;
	}

	//FResponse_Login LoginResponse;
	//GetStructFromJsonString<FResponse_Login>(Response, LoginResponse);

	//UE_LOG(LogTemp, Warning, TEXT("Id is: %d"), LoginResponse.id);
	//UE_LOG(LogTemp, Warning, TEXT("Name is: %s"), *LoginResponse.name);
	*/
}