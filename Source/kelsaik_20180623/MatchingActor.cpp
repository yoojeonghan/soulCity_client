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

	// ������ ������ JsonObject�� �����մϴ�.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// ������ JsonObject�� �޽����� ������ȣ�� 
	ChatObject->SetStringField("RequestNum", "2001");
	// �޽����� �۽��ϴ� ������ �߰��մϴ�.
	ChatObject->SetStringField("RequestUser", PlayerID);
	// �޽��� ������ �߰��մϴ�.
	//ChatObject->SetStringField("RequestString", sendtext.ToString());

	// JsonObject�� String���� ��ȯ�� �ڷ���
	FString OutputChatting;

	// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

	// JsonObject�� ����ȭ�մϴ�.
	FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

	// ���� Byte�� ũ�⸦ ��� ���� ����
	int32 BytesSent = 0;

	OutputChatting += "}";

	// ���� String�� UTF8�� ���ڵ��մϴ�. 
	FTCHARToUTF8 StringtoUTF8(*OutputChatting);

	// ������ String�� ������ �����մϴ�.
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
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

		// ������ JsonObject�� �޽����� ������ȣ�� 
		ChatObject->SetStringField("RequestNum", "1000");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		ChatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString OutputChatting;

		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

		// ���� Byte�� ũ�⸦ ��� ���� ����
		int32 BytesSent = 0;

		// ���� String�� UTF8�� ���ڵ��մϴ�. 
		FTCHARToUTF8 StringtoUTF8(*OutputChatting);

		// ������ String�� ������ �����մϴ�.
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

		// ������ ������ �����մϴ�.
		memcpy(RecvData, ReceivedData.GetData(), ReceivedData.Num());
		RecvData[ReceivedData.Num()] = 0;

		// ������ ������ String�� ����ϴ�.
		RecvString = ANSI_TO_TCHAR(RecvData);
		// ������ String�� JSONObject�� ��ȯ�մϴ�.
		TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);
		// Reader�� ���� JSONObject�� ���� �� �ֵ��� ��ȯ�մϴ�.
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(RecvString);

		// ����ȭ�� JSONObject�� �����·� ������ŵ�ϴ�.
		if (FJsonSerializer::Deserialize(Reader, RecvJsonObject))
		{
			// ���信 ���� ������ȣ�� ����ϴ�.
			AnswerNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("AnswerNum")));

			// ���信 ���� �ٸ� �Լ��� ȣ���մϴ�.
			switch (AnswerNum)
			{
				case 1001: // �α��� ����
					RoomNumber = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("RoomNumber")));
					break;

				case 2002:// ��Ī �Ϸ�
					RecevingMatch();
					break;
			
				default: // �̸� ���ǵ��� ���� ������ȣ ���� �� ȣ��
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

	// ������ ������ JsonObject�� �����մϴ�.
	TSharedPtr<FJsonObject> ChatObject = MakeShareable(new FJsonObject);

	// ������ JsonObject�� �޽����� ������ȣ�� 
	ChatObject->SetStringField("RequestNum", "2002");
	// �޽����� �۽��ϴ� ������ �߰��մϴ�.
	ChatObject->SetStringField("RequestUser", PlayerID);
	// �޽��� ������ �߰��մϴ�.
	//ChatObject->SetStringField("RequestString", sendtext.ToString());

	// JsonObject�� String���� ��ȯ�� �ڷ���
	FString OutputChatting;

	// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputChatting);

	// JsonObject�� ����ȭ�մϴ�.
	FJsonSerializer::Serialize(ChatObject.ToSharedRef(), Writer);

	// ���� Byte�� ũ�⸦ ��� ���� ����
	int32 BytesSent = 0;

	// ���� String�� UTF8�� ���ڵ��մϴ�. 
	FTCHARToUTF8 StringtoUTF8(*OutputChatting);

	// ������ String�� ������ �����մϴ�.
	Socket->SendTo((uint8*)StringtoUTF8.Get(), StringtoUTF8.Length(), BytesSent, *addr);
}

void AMatchingActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Socket->Close();
}