// Fill out your copyright notice in the Description page of Project Settings.

#include "TCPActor.h"


// Sets default values
ATCPActor::ATCPActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATCPActor::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerHUDClass = AHUD::StaticClass();

	if (GetWorld())
	{
		FString sessiontemp;
		bool IsSession = true;
		FFileHelper::LoadFileToString(sessiontemp, *(FPaths::ProjectDir() + FString("GameSession")));
		CharID = sessiontemp;

		SetTCPSocket();
	}
}


void ATCPActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*
	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// 생성한 JsonObject에 메시지의 고유번호와 
	ChatObject->SetStringField("RequestNum", "2000");
	// 메시지를 송신하는 유저를 추가합니다.
	ChatObject->SetStringField("RequestUser", FString::FromInt(RandCharNum));

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
	*/
}

void ATCPActor::SessionLogout()
{
	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// 생성한 JsonObject에 메시지의 고유번호와 
	ChatObject->SetStringField("RequestNum", "2000");
	// 메시지를 송신하는 유저를 추가합니다.
	ChatObject->SetStringField("RequestUser", CharID);

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

// Called every frame
void ATCPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATCPActor::SetTCPSocket()
{
	UMyGameInstance* myGameInstance = Cast<UMyGameInstance>(GetGameInstance());

	//FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address ip(52, 78, 90, 90);
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(801);
	//addr->SetPort(9006);

	bool connected = Socket->Connect(*addr);

	if (connected)
	{
		GetWorldTimerManager().SetTimer(TCPReceiverHandle, this, &ATCPActor::TCPSocketListener, 0.01f, true);
	}

	if (myGameInstance->IsTCPLogin == 0)
	{
		if (connected)
		{
			// 서버로 전송할 JsonObject를 생성합니다.
			TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

			// 생성한 JsonObject에 메시지의 고유번호와 
			ChatObject->SetStringField("RequestNum", "900");
			// 메시지를 송신하는 유저를 추가합니다.
			ChatObject->SetStringField("RequestUser", CharID);

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

			myGameInstance->IsTCPLogin = 1;
		}
	}

}

void ATCPActor::SetTCPConnection()
{

}

void ATCPActor::TCPSocketListener()
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

		RecevingChat(StringFromBinaryArray(ReceivedData));
		free(RecvData);
	}
}

void ATCPActor::SendingChat(FText sendtext)
{
	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// 일반채팅
	// 생성한 JsonObject에 메시지의 고유번호와 
	ChatObject->SetStringField("RequestNum", "1000");
	// 메시지를 송신하는 유저를 추가합니다.
	ChatObject->SetStringField("RequestUser", CharID);
	// 메시지 내용을 추가합니다.
	ChatObject->SetStringField("RequestString", sendtext.ToString());

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

void ATCPActor::RecevingChat(FString receivedString)
{
	Akelsaik_20180623GameMode * MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());

	// 수신한 String을 JSONObject로 변환합니다.
	TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);

	// Reader를 통해 JSONObject를 읽을 수 있도록 변환합니다.

	//FTCHARToUTF8 StringTOUTF82(*receivedString);

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(receivedString);

	// 직렬화된 JSONObject을 원상태로 복구시킵니다.
	if (FJsonSerializer::Deserialize(Reader, RecvJsonObject))
	{
		AnswerNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("AnswerNum")));
		RequestUser = *RecvJsonObject->GetStringField(TEXT("RequestUser"));
		AnswerString = *RecvJsonObject->GetStringField(TEXT("AnswerString"));
		//FTCHARToUTF8 StringTOUTF8(*AnswerString);

		switch (AnswerNum)
		{
		case 900: // 로그인
			MyGameMode->FLoginUser(RequestUser);
			break;
			//일반채팅
		case 1000:
			MyGameMode->FReceivedChatting(AnswerString, RequestUser);
			break;
			//전체채팅
		case 1001:
			break;
			//귓속말
		case 1002:
			break;
		case 1003:
			break;
		case 1004:
			break;
		case 2000: // 로그아웃
			MyGameMode->FLogoutUser(RequestUser);
			break;
		}
	}
}

FString ATCPActor::StringFromBinaryArray(TArray<uint8>& BinaryArray)
{
	BinaryArray.Add(0);
	FString Result = FString(UTF8_TO_TCHAR(BinaryArray.GetData()));
	return Result;
}
