// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionActor.h"

/*

EditText의 아이디값을 받아서 서버에 요청할 것.
서버에서 아이디가 있다는 메시지를 받을 것.
메시지를 받은 후 해당 아이디를 세션에 넣을 것(랜덤값을 대체할 것)

*/

// Sets default values
ASessionActor::ASessionActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASessionActor::BeginPlay()
{
	Super::BeginPlay();

	//int32 RandCharNum = FMath::RandRange(0, 10000);

	/*------------------------------------------------------------------------------------------------*/

	Http = &FHttpModule::Get();

	// You can uncomment this out for testing.
	//FRequest_Login LoginCredentials;
	//LoginCredentials.email = TEXT("asdf@asdf.com");
	//LoginCredentials.password = TEXT("asdfasdf");
	//Login(LoginCredentials);
}

// Called every frame
void ASessionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASessionActor::SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request)
{
	Request->SetHeader(AuthorizationHeader, Hash);
}

TSharedRef<IHttpRequest> ASessionActor::RequestWithRoute(FString Subroute)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(ApiBaseUrl + Subroute);
	SetRequestHeaders(Request);
	return Request;
}

void ASessionActor::SetRequestHeaders(TSharedRef<IHttpRequest>& Request)
{
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

TSharedRef<IHttpRequest> ASessionActor::GetRequest(FString Subroute)
{
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetVerb("GET");
	return Request;
}


TSharedRef<IHttpRequest> ASessionActor::PostRequest(FString Subroute, FString ContentJsonString)
{
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetVerb("POST");
	Request->SetContentAsString(ContentJsonString);
	return Request;
}

void ASessionActor::Send(TSharedRef<IHttpRequest>& Request)
{
	Request->ProcessRequest();
}

bool ASessionActor::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		return false;
	}
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		return true;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
		return false;
	}
}

template<typename StructType>
inline void ASessionActor::GetJsonStringFromStruct(StructType FilledStruct, FString& StringOutput)
{
	FJsonObjectConverter::UStructToJsonObjectString(StructType::StaticStruct(), &FilledStruct, StringOutput, 0, 0);
}

template<typename StructType>
inline void ASessionActor::GetStructFromJsonString(FHttpResponsePtr Response, StructType& StructOutput)
{
	StructType StructData;
	FString JsonString = Response->GetContentAsString();
	FJsonObjectConverter::JsonObjectStringToUStruct<StructType>(JsonString, &StructOutput, 0, 0);
}

/*

void ASessionActor::Login(FRequest_Login LoginCredentials)
{
	FString ContentJsonString;
	GetJsonStringFromStruct<FRequest_Login>(LoginCredentials, ContentJsonString);

	TSharedRef<IHttpRequest> Request = PostRequest("user/login", ContentJsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &ASessionActor::LoginResponse);
	Send(Request);
}

void ASessionActor::LoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!ResponseIsValid(Response, bWasSuccessful))
	{
		return;
	}

	FResponse_Login LoginResponse;
	GetStructFromJsonString<FResponse_Login>(Response, LoginResponse);

	UE_LOG(LogTemp, Warning, TEXT("Id is: %d"), LoginResponse.id);
	UE_LOG(LogTemp, Warning, TEXT("Name is: %s"), *LoginResponse.name);
}
*/