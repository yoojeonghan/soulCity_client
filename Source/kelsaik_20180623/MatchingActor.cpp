// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchingActor.h"


// Sets default values
AMatchingActor::AMatchingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MyCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("My Sphere Component"));
	MyCollisionSphere->InitSphereRadius(SphereRadius);
	MyCollisionSphere->SetCollisionProfileName("Trigger");

	MyCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMatchingActor::BeginOverlap);

	//IsChangeMap = false;
}

// Called when the game starts or when spawned
void AMatchingActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());
		//RandCharNum = MyGameMode->RandCharNum;

		FString sessiontemp;
		bool IsSession = true;
		FFileHelper::LoadFileToString(sessiontemp, *(FPaths::ProjectDir() + FString("GameSession")));
		PlayerID = sessiontemp;

		SetTCPSocket();

		//TimerDel.BindUFunction(this, FName("RecevingMatch"), 0);
		//GetWorldTimerManager().SetTimer(BeginMapTimerHandel, TimerDel, 10.0f, false);
	}
}

// Called every frame
void AMatchingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMatchingActor::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->ActorHasTag(FName("UserCharacter")))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("[MatcingActor] Begin Matching"));
		}
	}
}

void AMatchingActor::RequestMatching()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MatcingActor] Request Matching"));
	}

	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// 생성한 JsonObject에 메시지의 고유번호와 
	ChatObject->SetStringField("RequestNum", "2001");
	// 메시지를 송신하는 유저를 추가합니다.
	ChatObject->SetStringField("RequestUser", PlayerID);
	// 메시지 내용을 추가합니다.
	//ChatObject->SetStringField("RequestString", sendtext.ToString());

	// JsonObject를 String으로 변환할 자료형
	FString OutputChatting;

	// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

	// JsonObject를 직렬화합니다.
	FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

	// 보낼 Byte의 크기를 담기 위한 변수
	int32 BytesSent = 0;

	OutputChatting += "}";

	// 보낼 String을 UTF8로 인코딩합니다. 
	FTCHARToUTF8 StringtoUTF8(*OutputChatting);

	// 생성된 String을 서버로 전송합니다.
	Socket->SendTo((uint8*)StringtoUTF8.Get(), StringtoUTF8.Length(), BytesSent, *addr);
}

void AMatchingActor::SetTCPSocket()
{
	FIPv4Address ip(52, 78, 90, 90);
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(802);

	bool connected = Socket->Connect(*addr);

	if (connected)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

		// 생성한 JsonObject에 메시지의 고유번호와 
		ChatObject->SetStringField("RequestNum", "1000");
		// 메시지를 송신하는 유저를 추가합니다.
		ChatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString OutputChatting;

		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

		// 보낼 Byte의 크기를 담기 위한 변수
		int32 BytesSent = 0;

		// 보낼 String을 UTF8로 인코딩합니다. 
		FTCHARToUTF8 StringtoUTF8(*OutputChatting);

		// 생성된 String을 서버로 전송합니다.
		Socket->SendTo((uint8*)StringtoUTF8.Get(), StringtoUTF8.Length(), BytesSent, *addr);

		GetWorldTimerManager().SetTimer(TCPReceiverHandle, this, &AMatchingActor::TCPSocketListener, 0.01f, true);
	}
}

void AMatchingActor::SetTCPConnection()
{

}

void AMatchingActor::TCPSocketListener()
{
	TArray<uint8> ReceivedData;

	uint32 Size;

	while (Socket->HasPendingData(Size))
	{
		uint8 element = 0;
		ReceivedData.Init(element, FMath::Min(Size, 65507u));

		int32 Read = 0;
		Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		if (ReceivedData.Num() <= 0)
		{
			return;
		}

		RecvData = (char*)malloc(ReceivedData.Num() + 1);

		// 버퍼의 내용을 복사합니다.
		memcpy(RecvData, ReceivedData.GetData(), ReceivedData.Num());
		RecvData[ReceivedData.Num()] = 0;

		// 버퍼의 내용을 String에 담습니다.
		RecvString = ANSI_TO_TCHAR(RecvData);
		// 수신한 String을 JSONObject로 변환합니다.
		TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);
		// Reader를 통해 JSONObject를 읽을 수 있도록 변환합니다.
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(RecvString);

		// 직렬화된 JSONObject을 원상태로 복구시킵니다.
		if (FJsonSerializer::Deserialize(Reader, RecvJsonObject))
		{
			// 응답에 대한 고유번호를 얻습니다.
			AnswerNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("AnswerNum")));

			// 응답에 따라 다른 함수를 호출합니다.
			switch (AnswerNum)
			{
				case 1001: // 로그인 응답
					RoomNumber = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("RoomNumber")));
					break;

				case 2002:// 매칭 완료
					RecevingMatch();
					break;
			
				default: // 미리 정의되지 않은 고유번호 수신 시 호출
					if (GEngine)
					{
						FString LogString = "";
						LogString = TEXT("Defult Message from Server ::: ") + RecvString;
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, LogString);
					}
					break;
			}
		}
		free(RecvData);
	}
}

FString AMatchingActor::StringFromBinaryArray(TArray<uint8>& BinaryArray)
{
	BinaryArray.Add(0);
	FString Result = FString(UTF8_TO_TCHAR(BinaryArray.GetData()));
	return Result;
}

void AMatchingActor::RecevingMatch()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MatcingActor] Matching Success! "));
		MyGameMode->RecevingMatch();
	}
}

void AMatchingActor::CanselMatching()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MatcingActor] Cansel Matching"));
	}

	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// 생성한 JsonObject에 메시지의 고유번호와 
	ChatObject->SetStringField("RequestNum", "2002");
	// 메시지를 송신하는 유저를 추가합니다.
	ChatObject->SetStringField("RequestUser", PlayerID);
	// 메시지 내용을 추가합니다.
	//ChatObject->SetStringField("RequestString", sendtext.ToString());

	// JsonObject를 String으로 변환할 자료형
	FString OutputChatting;

	// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

	// JsonObject를 직렬화합니다.
	FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

	// 보낼 Byte의 크기를 담기 위한 변수
	int32 BytesSent = 0;

	// 보낼 String을 UTF8로 인코딩합니다. 
	FTCHARToUTF8 StringtoUTF8(*OutputChatting);

	// 생성된 String을 서버로 전송합니다.
	Socket->SendTo((uint8*)StringtoUTF8.Get(), StringtoUTF8.Length(), BytesSent, *addr);
}

void AMatchingActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Socket->Close();
}