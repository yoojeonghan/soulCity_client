// Fill out your copyright notice in the Description page of Project Settings.

#include "UDPActor.h"
#include "AES.h"
#include "string.h"
#include "ThirdParty/CryptoPP/5.6.5/include/aes.h"
#include "ThirdParty/CryptoPP/5.6.5/include/hex.h"
#include "ThirdParty/CryptoPP/5.6.5/include/modes.h"

using namespace std;
using namespace CryptoPP;

// UDPActor�� ������. 
// �������� ���� ������ �ʱ�ȭ�ϸ� tick()�Լ��� ȣ�� �󵵸� �����մϴ�. 
AUDPActor::AUDPActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ĳ���� ���� �� �浹������ ���� �ʱ� ���� ����
	bNoCollisionFail = true;
	// ĳ���� ���� ���� �̸� ����
	SpawnInfo.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// ĳ���� ���� �� ����� Vector�� Rotator �ʱ�ȭ
	myLoc = FVector(-700.0f, 370.0f, 227.0f);
	myRot = FRotator(0, 0, 0);

	// �̸� ������ ĳ���� ����
	CharNum = 10;

	MonsterStateNum = 0;

	IsTeamUI = false;

	//MyAES = new FAES();
}

// Called when the game starts or when spawned
void AUDPActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());

		// ���� ���� �� CharNum ���� ���� ĳ���͸� �����ϰ� ���� ��Ȱ��ȭ��ŵ�ϴ�. (�Ŀ� ������ ���� ���� ���� ��Ȱ��ȭ�� ĳ���͸� Ȱ��ȭ��ŵ�ϴ�.)
		for (int32 i = 0; i < CharNum; i++)
		{
			// ������ ĳ���� ���͸� WorldCharacter ����Ʈ�� �ֽ��ϴ�. 
			// WorldCharacter.Add(GetWorld()->SpawnActor<AWorldCharacter>(AWorldCharacter::StaticClass, myLoc, myRot, SpawnInfo));

			for (TActorIterator<AWorldCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				WorldCharacter.Add(*ActorItr);
			}
		}

		FString sessiontemp;
		bool IsSession = true;
		FFileHelper::LoadFileToString(sessiontemp, *(FPaths::ProjectDir() + FString("GameSession")));
		PlayerID = sessiontemp;

		for (TActorIterator<AMonsterCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			MonsterCharacter = *ActorItr;
		}

		for (TActorIterator<AItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			ItemActor = *ActorItr;
		}

	}

	// ������ �������� �ʾ��� ��
	if (!IsHeart)
	{
		// ���� ������ �� �� �����մϴ�.

		// ��Ʈ��Ʈ�� �۽��ϴ� Ÿ�� �ڵ鷯
		GetWorldTimerManager().SetTimer(HeartBeatSenderHandle, this, &AUDPActor::HeartBeatSender, 1.0f, true);
		// ��Ʈ��Ʈ�� ���� ���θ� ���� �ֱ⸶�� �ʱ�ȭ��Ű�� Ÿ�� �ڵ鷯
		GetWorldTimerManager().SetTimer(HeartBeatReceverHandle, this, &AUDPActor::HeartBeatRecever, 4.2f, true);
		// ��Ʈ��Ʈ�� ���� �ֱ⸶�� ���ŵ��� ������ Ŭ���̾�Ʈ�� �����ϴ� Ÿ�� �ڵ鷯
		//GetWorldTimerManager().SetTimer(ConnectManagerHandle, this, &AUDPActor::UnConnectDistroyGame, 7.777f, true);
		IsHeart = true;
	}

	
}

// UDP ���Ͱ� �ı��� �� �ҷ����� �Լ�
// ������ �α׾ƿ��� ��û�մϴ�.
void AUDPActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

// Called every frame
void AUDPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// UDP������ �����ϴ� �Լ�. 
// ���� ������ ��� �Ϸ�Ǹ� ToServerLocation()�� ȣ���մϴ�.
void AUDPActor::SetUdpSocket()
{
	UMyGameInstance* myGameInstance = Cast<UMyGameInstance>(GetGameInstance());

	// �۽��� ������ IP�� �����մϴ�.
	//FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address ip(52, 78, 90, 90);

	// InternetAddress�� �����մϴ�.
	addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	// ������ IP�� �����մϴ�.
	addr->SetIp(ip.Value);
	// ������ ��Ʈ�� �����մϴ�.
	addr->SetPort(800);

	// UDP ������ �����մϴ�.
	Socket = FUdpSocketBuilder("UDPSocket").AsReusable().WithBroadcast();
	// ���� ���� ����� ���մϴ�.
	int32 BufferSize = 2 * 1024 * 1024;

	// ������ ���� ���� ����� �����մϴ�.
	Socket->SetSendBufferSize(BufferSize, BufferSize);
	Socket->SetReceiveBufferSize(BufferSize, BufferSize);

	// ������ ������ �� bool������ ������� �ֽ��ϴ�.
	bool connected = Socket->Connect(*addr);

	/*-----------------------------------------------------------------------------------------------------*/

	if (myGameInstance->IsLogin == 0)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> LoginObject = MakeShareable(new FJsonObject);

		// ������ JsonObject�� �޽����� ������ȣ�� 
		LoginObject->SetStringField("RequestNum", "1001");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		LoginObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString OutputLogin;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(LoginObject.ToSharedRef(), Writer);

		myGameInstance->IsLogin = 1;

		AESEncryption(OutputLogin);
	}

	// ������ ����Ǿ��� ���
	if (connected)
	{
		// UDP Receiver�� ���ð��� �����մϴ�.
		FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
		// UDP Receiver�� �����մϴ�.
		UDPReceiver = new FUdpSocketReceiver(Socket, ThreadWaitTime, TEXT("UDP_RECEIVER"));
		// UDP Receiver�� �޽����� �޴� �Լ��� �����մϴ�.
		UDPReceiver->OnDataReceived().BindUObject(this, &AUDPActor::RecvFromServer);
		// UDP Receiver�� �����մϴ�.
		UDPReceiver->Start();

		// ������ �����ǰ� ������ ������ ��, ���ù��� ����Ǹ� ���� ���¸� �����ϴ�.
		ToServerState(1);
		ToServerState(2);

		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> LoginObject = MakeShareable(new FJsonObject);

		// ������ JsonObject�� �޽����� ������ȣ�� 
		LoginObject->SetStringField("RequestNum", "1003");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		LoginObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString OutputLogin;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(LoginObject.ToSharedRef(), Writer);

		if (connected)
		{
			AESEncryption(OutputLogin);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[UDPActor] Connected UDP Netty Server Faild!"));
		}
	}
}

// �����κ��� �޽����� �޴� �Լ�
void AUDPActor::RecvFromServer(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	// �޽����� ������ ������ ����� �������� �Ҵ��մϴ�.
	RecvData = (char*)malloc(ArrayReaderPtr->Num() + 1);

	// ������ ������ �����մϴ�.
	memcpy(RecvData, ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	RecvData[ArrayReaderPtr->Num()] = 0;
	// ������ ������ String�� ����ϴ�.
	RecvString = ANSI_TO_TCHAR(RecvData);

	// ��ȣȭ
	FString AESKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	bool IsDecrypted = false;
	RecvString = CBC_AESDecryptData(RecvString, AESKey, "ABCDEF0123456789", IsDecrypted);
	
	if (IsDecrypted)
	{
		// ������ String�� JSONObject�� ��ȯ�մϴ�.
		TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);
		// Reader�� ���� JSONObject�� ���� �� �ֵ��� ��ȯ�մϴ�.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RecvString);

		// ����ȭ�� JSONObject�� �����·� ������ŵ�ϴ�.
		if (FJsonSerializer::Deserialize(Reader, RecvJsonObject))
		{
			// ���信 ���� ������ȣ�� ����ϴ�.
			AnswerNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("AnswerNum")));

			// ������ ANS_CURRENTUSER(1120) �Ǵ� CHARMOVE(1121)�� ��� RequestUser�� ���� �ʽ��ϴ�.
			if (AnswerNum != 1121 && AnswerNum != 1120)
			{
				RequestUser = *RecvJsonObject->GetStringField(TEXT("RequestUser"));
			}

			// ���信 ���� �ٸ� �Լ��� ȣ���մϴ�.
			switch (AnswerNum)
			{
			case 1000: // HEART_BEAT ��Ʈ��Ʈ
				Recv_HeartBeat();
				break;

			case 1120: //ANS_CURRENTUSER ���� ĳ���� ��� ����
				Recv_CurrentUser(RecvJsonObject);
				break;

			case 1171: // CHARSTATE ĳ���� ���� ����
				Recv_CharState(RequestUser, RecvJsonObject);
				break;

			case 2001: //YES LOGON �α��� ����
				Recv_LogIn();
				break;

			case 2002: //YES LOGOUT �α׾ƿ� ����
				Recv_LogOut();
				break;

			case 3031:
				if (ItemActor)
				{
					ItemActor->NonVisibleItem();
				}
				break;

			case 4000:
				MonsterStateNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("MonsterState")));
				Recv_MonsterState(MonsterStateNum, RecvJsonObject);
				break;

			case 5001:
				if (RequestUser.Equals(PlayerID))
				{
					MyGameMode->UpdateMyHPBar(FCString::Atod(*RecvJsonObject->GetStringField(TEXT("RequestUserHP"))));
				}
				else
				{
					MyGameMode->UpdateUserHPBar(RequestUser, FCString::Atod(*RecvJsonObject->GetStringField(TEXT("RequestUserHP"))));
				}
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

		// �������� �Ҵ��� ���۸� �����մϴ�.
		free(RecvData);
	}
	else
	{
		free(RecvData);
	}

}

// ��Ʈ��Ʈ�� �۽��ϴ� �Լ�
void AUDPActor::HeartBeatSender()
{
	if (!IsSocket)
	{
		SetUdpSocket();
		IsSocket = true;
	}

	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "1000");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString HeartBeat;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&HeartBeat);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(HeartBeat);
	}

}

// ��Ʈ��Ʈ ���� ���θ� ���� �ֱ⸶�� �ʱ�ȭ��Ű�� �Լ�
void AUDPActor::HeartBeatRecever()
{
	IsConnect = false;
}

// ��Ʈ��Ʈ�� ���� �ֱ⸶�� ���ŵ��� ������ Ŭ���̾�Ʈ�� �����ϴ� �Լ�
void AUDPActor::UnConnectDistroyGame()
{
	if (!IsConnect)
	{
		Dialog.Open(EAppMsgType::Ok, FText::FromString(TEXT("Network connection Fail.")));
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}
}

// ��Ʈ��Ʈ�� �����ϴ� �Լ�
void AUDPActor::Recv_HeartBeat()
{
	IsConnect = true;
}

// �����κ��� ���� ������ ���� ����� �޾� ĳ������ Ȱ��ȭ & ��Ȱ��ȭ ���θ� �����ϴ� �Լ�
void AUDPActor::Recv_CurrentUser(TSharedPtr<FJsonObject> RecvJsonObject)
{
	if (GEngine)
	{
		// �̹� Ȱ��ȭ�Ǿ��ִ� ���� ����� ��� Array
		TArray<FString> LastPlayerNameArray = PlayerNameArray;
		// ���ο� ĳ���͸� �Ҵ�޾ƾ� �ϴ� ���� ����� ��� Array
		TArray<FString> NewPlayerNameArray;
		// �Ҵ�� ĳ���͸� �����ؾ� �ϴ� ���� ����� ��� Array
		TArray<FString> DeletePlayerNameArray;

		// ���� ������ �������� �г����� �����ϴ� Array. �� �迭�� �ε��� ������ ���� ĳ���͸� Ȱ��ȭ��ŵ�ϴ�.
		// Array�� �ʱ�ȭ�մϴ�.
		PlayerNameArray.Empty();

		// �����κ��� PlayerList �迭�� �޽��ϴ�. {"PlayerList":["941", "941", "941", "941", "941", "941", "941"],"AnswerNum":1120}
		TArray<TSharedPtr<FJsonValue>> objArray = RecvJsonObject->GetArrayField(TEXT("PlayerList"));

		// PlayerNameArray�� �ڽ��� ������ ������ ��� �������� �߰��մϴ�.
		for (int32 i = 0; i < objArray.Num(); i++)
		{
			FString playerName = objArray[i]->AsString();
			if (!playerName.Equals(PlayerID))
			{
				PlayerNameArray.Add(playerName);
			}
		}

		// PlayerNameArray�� �迭 �� ��ŭ �ݺ��� ������ ������ ĳ���Ͱ� Ȱ��ȭ�� ���������� Ȯ��
		for (int32 m = 0; m < PlayerNameArray.Num(); m++)
		{
			if (!LastPlayerNameArray.Contains(PlayerNameArray[m]))
			{
				NewPlayerNameArray.Add(PlayerNameArray[m]);
			}
		}


		// ĳ���Ͱ� Ȱ��ȭ���� ���� ������� �� ĳ���� �Ҵ�
		for (int32 n = 0; n < NewPlayerNameArray.Num(); n++)
		{
			bool IsCharacter = false;

			for (int32 s = 0; s < WorldCharacter.Num(); s++)
			{
				if (!IsCharacter)
				{
					if (WorldCharacter[s])
					{
						if (WorldCharacter[s]->CharID.IsEmpty())
						{
							WorldCharacter[s]->SetcharID(NewPlayerNameArray[n]);
							WorldCharacter[s]->IsWorld = false;
							WorldCharacter[s]->IsBegining = true;
							IsCharacter = true;
						}
						else
						{
							if (GEngine)
							{
								//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[UDPActor] WorldCharacter[index] is NULL"));
							}
						}
					}

				}
				else
				{
					break;
				}
			}
		}

		if (MonsterCharacter && !IsTeamUI)
		{
			MyGameMode->CreateTeamUI(NewPlayerNameArray);
			IsTeamUI = true;
		}

		// LastPlayerNameArray�� �迭 �� ��ŭ �ݺ��� ������ ĳ���Ͱ� Ȱ��ȭ�� ������ ���絵 �����ִ��� Ȯ��
		for (int32 p = 0; p < LastPlayerNameArray.Num(); p++)
		{
			if (!PlayerNameArray.Contains(LastPlayerNameArray[p]))
			{
				DeletePlayerNameArray.Add(LastPlayerNameArray[p]);
			}
		}

		// ����� ������ ĳ���ʹ� ��Ȱ��ȭ ��Ŵ
		for (int32 j = 0; j < DeletePlayerNameArray.Num(); j++)
		{
			bool DeleteCharacter = false;

			for (int32 v = 0; v < WorldCharacter.Num(); v++)
			{
				if (!DeleteCharacter)
				{
					if (WorldCharacter[v])
					{
						if (WorldCharacter[v]->CharID.Equals(DeletePlayerNameArray[j]))
						{
							WorldCharacter[v]->SetcharID("");
							DeleteCharacter = true;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
	}
}

// ĳ������ ���¸� �۽��ϴ� �Լ�
void AUDPActor::ToServerState(int32 stateNum)
{
	if (GetWorld())
	{
		// ���°��� ���� JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> StateObject = MakeShareable(new FJsonObject);

		// ������ JsonObject�� �޽����� ������ȣ�� 
		StateObject->SetStringField("RequestNum", "1071");

		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		StateObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� ���°��� �߰��մϴ�.
		StateObject->SetStringField("StateNum", FString::FromInt(stateNum));

		if (stateNum == 1)
		{
			// ���忡�� ĳ������ ��Ʈ�ѷ��� �޽��ϴ�.
			myCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld()->GetFirstPlayerController(), 0);

			if (myCharacter)
			{
				// Vector�� �� ĳ������ ��ġ�� �޽��ϴ�.
				myLocation = myCharacter->GetActorLocation();

				// Rotator�� �� ĳ������ ��ġ�� �޽��ϴ�.
				myRotation = myCharacter->GetActorRotation();

				// �� ������ Vector�� X,Y,Z ���� �����մϴ�.
				myLocationX = FString::SanitizeFloat(myLocation.X);
				myLocationY = FString::SanitizeFloat(myLocation.Y);
				myLocationZ = FString::SanitizeFloat(myLocation.Z);
				myRotationYaw = FString::SanitizeFloat(myRotation.Yaw);

				// JsonObject�� ��ǥ���� �߰��մϴ�.
				StateObject->SetStringField("LocationX", myLocationX);
				StateObject->SetStringField("LocationY", myLocationY);
				StateObject->SetStringField("LocationZ", myLocationZ);

				// JsonObject�� ȸ������ �߰��մϴ�.
				StateObject->SetStringField("RotationYaw", myRotationYaw);
			}
		}

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString OutputState;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputState);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(StateObject.ToSharedRef(), Writer);

		AESEncryption(OutputState);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[UDPActor] GetWorld() is NULL"));
		}
	}
}

// �α����� �����ϴ� �Լ�
void AUDPActor::Recv_LogIn()
{
}

// �α׾ƿ��� �����ϴ� �Լ�
void AUDPActor::Recv_LogOut()
{
}

void AUDPActor::SessionLogout()
{
	// ������ ������ JsonObject�� �����մϴ�.
	TSharedPtr<FJsonObject> LogoutObject = MakeShareable(new FJsonObject);

	// ������ JsonObject�� �޽����� ������ȣ�� 
	LogoutObject->SetStringField("RequestNum", "1002");
	// �޽����� �۽��ϴ� ������ �߰��մϴ�.
	LogoutObject->SetStringField("RequestUser", PlayerID);

	// JsonObject�� String���� ��ȯ�� �ڷ���
	FString OutputLogin;
	// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
	// JsonObject�� ����ȭ�մϴ�.
	FJsonSerializer::Serialize(LogoutObject.ToSharedRef(), Writer);

	AESEncryption(OutputLogin);
}

// �����κ��� ���� �޽����� ���� �� ó���ϴ� �Լ�
void AUDPActor::Recv_CharState(FString requestUser, TSharedPtr<FJsonObject> RecvJsonObject)
{
	//Server sending response :{"StateNum":"1","AnswerNum":1171,"RequestUser":"8432"}
	// PlayerName�� PlayerObj�� Ű�� �����մϴ�.

	FString PlayerName = requestUser;

	TSharedPtr<FJsonObject> PlayerStateObj = RecvJsonObject;

	int32 StateNum = FCString::Atoi(*PlayerStateObj->GetStringField("StateNum"));

	if (StateNum == 1)
	{
		// PlayerLocationobj���� X,Y,Z ��ǥ�� Y�� ȸ������ ����ϴ�.
		UPROPERTY(EditAnywhere)
		float playerX;
		UPROPERTY(EditAnywhere)
		float playerY;
		UPROPERTY(EditAnywhere)
		float playerZ;
		UPROPERTY(EditAnywhere)
		float PlayerYaw;

		// playerX, playerY, playerZ�� �� float���� �ϳ��� Vector ������ ���� ���� �ڷ���
		FVector playerVector;

		// ȸ������ �ϳ��� ���� ���� �ڷ���
		FRotator playerRotator;

		if (GEngine)
		{
			// RecvMessage : 
			//{"AnswerNum":1121,"PlayerLocation":{"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"},"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}}}

			// Playerobj�� PlayerLocation�� ���� �����ɴϴ�.
			TSharedPtr<FJsonObject> Playerobj = RecvJsonObject->GetObjectField(TEXT("PlayerLocation"));

			//({"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"},"942":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}})

			// PlayerNameArray�� �� ��ŭ �ݺ��� ������ �ϸ� �ȵǰ�...
			// PlayerObject ����ŭ �ݺ��� ������ ������ �� ���̵� ���� �����ߵ�

			for (int32 i = 0; i < PlayerNameArray.Num(); i++)
			{
				FString PlayerName = PlayerNameArray[i];

				// PlayerLocationObj�� Playerobj���� PlayerName�� ���� LocationX, LocationY, LocationZ ������ JSONObject�� ��ü�� �����ɴϴ�. 
				//("LocationX" : "0.0")

				if (!PlayerName.Equals(PlayerID))
				{
					PlayerStateObj = Playerobj->GetObjectField(PlayerName);
					// "941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}

					if (PlayerStateObj.GetSharedReferenceCount() > 0)
					{
						// PlayerLocationobj���� X,Y,Z ��ǥ�� ����ϴ�.
						playerX = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationX")));
						playerY = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationY")));
						playerZ = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationZ")));

						// playerX, playerY, playerZ�� �� float���� �ϳ��� Vector ������ ���� ���� �ڷ���
						playerVector = FVector(playerX, playerY, playerZ);

						// PlayerLocationobj���� ȸ������ ����ϴ�.
						//playerPitch = FCString::Atof(*PlayerLocationobj->GetStringField(TEXT("RotationPitch")));
						//playerRoll = FCString::Atof(*PlayerLocationobj->GetStringField(TEXT("RotationRoll")));
						PlayerYaw = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("RotationYaw")));

						// ȸ������ �ϳ��� ���� ���� �ڷ���
						playerRotator = FRotator(0.0f, PlayerYaw, 0.0f);

						// �̸� ������ ĳ���� ������ �� ��ŭ �ݺ��� ����
						for (int32 j = 0; j < WorldCharacter.Num(); j++)
						{
							// WorldCharacter[i]�� �г����� �Ҵ�Ǿ����� �ʴٸ� �ݺ��� ����
							if (WorldCharacter[j]->CharID.IsEmpty())
							{
								break;
							}
							else
							{
								if (WorldCharacter[j]->CharID.Equals(PlayerName))
								{
									WorldCharacter[j]->MoveLocation(playerVector, playerRotator);
								}
							}
						}
					}
				}
			}
		}

		for (int32 j = 0; j < WorldCharacter.Num(); j++)
		{
			if (WorldCharacter[j]->CharID.Equals(PlayerName))
			{
				int32 TempState = WorldCharacter[j]->CharStateNum;
				WorldCharacter[j]->IsJog = true;
				WorldCharacter[j]->FUpdateState(StateNum);
			}
		}

	}
	else
	{
		// �̸� ������ ĳ���� ������ �� ��ŭ �ݺ��� ����
		for (int32 j = 0; j < WorldCharacter.Num(); j++)
		{
			if (WorldCharacter[j]->CharID.Equals(PlayerName))
			{
				int32 TempState = WorldCharacter[j]->CharStateNum;

				if (StateNum == 4)
				{
					if (!WorldCharacter[j]->CharacterMovementComponet->IsFalling())
					{
						WorldCharacter[j]->FUpdateState(StateNum);
					}
				}
				else if (StateNum == 6)
				{
					WorldCharacter[j]->IsAttack = true;
					WorldCharacter[j]->FUpdateState(StateNum);
				}
				else
				{
					if (TempState != StateNum)
					{
						WorldCharacter[j]->FUpdateState(StateNum);
					}
				}
			}
		}
	}
}

// ���� �ݰ�� �浹���� �� ���� Ȱ��ȭ�� ��û�ϴ� �Լ�
void AUDPActor::RequestMonsterBegin()
{
	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "3001");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString MosterReqStr;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

// ���� ���� ���� �� ���� ���� ������ ��û�ϴ� �Լ�
void AUDPActor::RequestMonsterAttack(float damage)
{
	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "3011");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// �������� �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString MosterReqStr;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

// ���� ���� ���� �� ���� ���� ������ ��û�ϴ� �Լ�
void AUDPActor::RequestMonsterAttack2(float damage)
{
	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "3012");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// �������� �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString MosterReqStr;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

void AUDPActor::RequestUserAttack(float damage)
{
	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "3021");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// �������� �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString MosterReqStr;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

void AUDPActor::Recv_MonsterState(int32 monsterStateNum, TSharedPtr<FJsonObject> recvJsonObject)
{
	double MonsterHP;
	double MonsterYaw;

	FVector MonsterLocation;
	FRotator MonsterRotator;
	
	double MonsterLocationX;
	double MonsterLocationY;
	double MonsterLocationZ;

	switch (monsterStateNum)
	{
	case 4001:
		MonsterCharacter->FUpdateState(2);
		break;

	case 4002:
		MonsterYaw = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterYaw")));
		MonsterRotator = FRotator(0, MonsterYaw, 0);
		MonsterCharacter->MonsterMove(MonsterRotator);
		MonsterCharacter->FUpdateState(6);
		break;

	case 4003:
		MonsterHP = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterHP")));

		if (MonsterHP > 0)
		{
			MyGameMode->UpdateMonsterHPBar(MonsterHP);
		}
		else
		{
			MyGameMode->VisibleMonsterUI(false);
		}
		break;

	case 4004:
		MonsterCharacter->FUpdateState(7);
		break;

	case 4005:
		MonsterLocationX = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterLocationX")));
		MonsterLocationY = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterLocationY")));
		MonsterLocationZ = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterLocationZ")));
		MonsterYaw = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterYaw")));

		MonsterLocation = FVector(MonsterLocationX, MonsterLocationY, MonsterLocationZ);
		MonsterRotator = FRotator(0, MonsterYaw + 50, 0);

		MonsterCharacter->MonsterMove(MonsterLocation, MonsterRotator);
		MonsterCharacter->FUpdateState(1);
		break;

	case 4006:
		MonsterHP = FCString::Atod(*recvJsonObject->GetStringField(TEXT("MonsterHP")));

		if (MonsterHP > 0)
		{
			MyGameMode->UpdateMonsterHPBar(MonsterHP);
			MonsterCharacter->FUpdateState(5);
		}
		else
		{
			MyGameMode->VisibleMonsterUI(false);
		}
		break;
	}
}

void AUDPActor::SendItemLooting()
{
	if (IsSocket)
	{
		// ������ ������ JsonObject�� �����մϴ�.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// ������ JsonObject�� �޽����� ������ȣ�� 
		HeartBeatObject->SetStringField("RequestNum", "3031");
		// �޽����� �۽��ϴ� ������ �߰��մϴ�.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject�� String���� ��ȯ�� �ڷ���
		FString MosterReqStr;
		// JsonWriter�� ���Ͽ� JsonObject�� String���� ��ȯ�մϴ�.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject�� ����ȭ�մϴ�.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);
		
		AESEncryption(MosterReqStr);
	}
}

void AUDPActor::AESEncryption(FString requestStr)
{
	FString AESKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	// Ű�� UTF8�� ���ڵ�
	TCHAR *KeyTChar = AESKey.GetCharArray().GetData();
	FTCHARToUTF8 StringtoUTF8_0(*AESKey);

	FString RequestStr = requestStr;
	// ���� Byte�� ũ�⸦ ��� ���� ����
	int32 BytesSent = 0;

	while (true)
	{
		FTCHARToUTF8 StringtoUTF8(*RequestStr);
		auto plainText = StringCast<ANSICHAR>(StringtoUTF8.Get());
		if (plainText.Length() % 16 != 0)
		{
			RequestStr.AppendChar(' ');
		}
		else
		{
			break;
		}
	}

	FString Encrypted = AUDPActor::CBC_AESEncryptData(RequestStr, StringtoUTF8_0.Get(), "ABCDEF0123456789");
	FTCHARToUTF8 StringtoUTF8_1(*Encrypted);

	Socket->SendTo((uint8*)StringtoUTF8_1.Get(), StringtoUTF8_1.Length(), BytesSent, *addr);
}

FString AUDPActor::ECB_AESEncryptData(FString aes_content, FString aes_key)
{
	std::string sKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	//std::string sKey = "0123456789ABCDEF0123456789ABCDEF";
	//std::string sKey = TCHAR_TO_UTF8(*aes_key);
	
	const char* plainText = TCHAR_TO_ANSI(*aes_content);
	std::string outstr;

	SecByteBlock key(AES::MAX_KEYLENGTH);
	memset(key, 0x30, key.size());
	sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);

	AES::Encryption aesEncryption((byte *)key, AES::MAX_KEYLENGTH);

	ECB_Mode_ExternalCipher::Encryption ecbEncryption(aesEncryption);
	StreamTransformationFilter ecbEncryptor(ecbEncryption, new HexEncoder(new StringSink(outstr)), BlockPaddingSchemeDef::BlockPaddingScheme::ONE_AND_ZEROS_PADDING, true);
	ecbEncryptor.Put((byte *)plainText, strlen(plainText));
	ecbEncryptor.MessageEnd();

	return UTF8_TO_TCHAR(outstr.c_str());
}

FString AUDPActor::ECB_AESDecryptData(FString aes_content, FString aes_key, bool & result)
{
	std::string sKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	const char* cipherText = TCHAR_TO_ANSI(*aes_content);
	std::string outstr;

	try
	{
		SecByteBlock key(AES::MAX_KEYLENGTH);
		memset(key, 0x30, key.size());
		sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);

		ECB_Mode<AES >::Decryption ecbDecryption((byte *)key, AES::MAX_KEYLENGTH);

		HexDecoder decryptor(new StreamTransformationFilter(ecbDecryption, new StringSink(outstr), BlockPaddingSchemeDef::BlockPaddingScheme::ONE_AND_ZEROS_PADDING, true));
		decryptor.Put((byte *)cipherText, strlen(cipherText));
		decryptor.MessageEnd();

		result = true;
	}
	catch (const std::exception&)
	{
		outstr = "error";
		UE_LOG(LogTemp, Error, TEXT("ECB_AESDecryptData failed!"));
		result = false;
	}

	return UTF8_TO_TCHAR(outstr.c_str());
}

FString AUDPActor::CBC_AESEncryptData(FString aes_content, FString aes_key, FString aes_IV)
{
	std::string sKey = TCHAR_TO_UTF8(*aes_key);
	std::string sIV = TCHAR_TO_UTF8(*aes_IV);
	auto plainText = StringCast<ANSICHAR>(*aes_content);
	std::string outstr;

	SecByteBlock key(AES::MAX_KEYLENGTH);
	memset(key, 0x30, key.size());
	sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);

	byte iv[AES::BLOCKSIZE];
	memset(iv, 0x30, AES::BLOCKSIZE);
	sIV.size() <= AES::BLOCKSIZE ? memcpy(iv, sIV.c_str(), sIV.size()) : memcpy(iv, sIV.c_str(), AES::BLOCKSIZE);

	AES::Encryption aesEncryption((byte *)key, AES::MAX_KEYLENGTH);
	CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	StreamTransformationFilter cbcEncryptor(cbcEncryption, new HexEncoder(new StringSink(outstr)), BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING, true);
	cbcEncryptor.Put((byte *)plainText.Get(), strlen(plainText.Get()));
	cbcEncryptor.MessageEnd();

	return UTF8_TO_TCHAR(outstr.c_str());
}

FString AUDPActor::CBC_AESDecryptData(FString aes_content, FString aes_key, FString aes_IV, bool & result)
{
	std::string sKey = TCHAR_TO_UTF8(*aes_key);
	std::string sIV = TCHAR_TO_UTF8(*aes_IV);
	auto cipherText = StringCast<ANSICHAR>(*aes_content);
	std::string outstr;

	try
	{
		SecByteBlock key(AES::MAX_KEYLENGTH);
		memset(key, 0x30, key.size());
		sKey.size() <= AES::MAX_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::MAX_KEYLENGTH);

		byte iv[AES::BLOCKSIZE];
		memset(iv, 0x30, AES::BLOCKSIZE);
		sIV.size() <= AES::BLOCKSIZE ? memcpy(iv, sIV.c_str(), sIV.size()) : memcpy(iv, sIV.c_str(), AES::BLOCKSIZE);

		CBC_Mode<AES >::Decryption cbcDecryption((byte *)key, AES::MAX_KEYLENGTH, iv);

		HexDecoder decryptor(new StreamTransformationFilter(cbcDecryption, new StringSink(outstr), BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING, true));
		decryptor.Put((byte *)cipherText.Get(), strlen(cipherText.Get()));
		decryptor.MessageEnd();

		result = true;
	}
	catch (const std::exception&)
	{
		outstr = "error";
		UE_LOG(LogTemp, Error, TEXT("CBC_AESDecryptData failed!"));
		result = false;
	}

	return UTF8_TO_TCHAR(outstr.c_str());

}