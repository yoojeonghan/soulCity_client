// Fill out your copyright notice in the Description page of Project Settings.

#include "UDPActor.h"
#include "AES.h"
#include "string.h"
#include "ThirdParty/CryptoPP/5.6.5/include/aes.h"
#include "ThirdParty/CryptoPP/5.6.5/include/hex.h"
#include "ThirdParty/CryptoPP/5.6.5/include/modes.h"

using namespace std;
using namespace CryptoPP;

// UDPActor의 생성자. 
// 전역으로 사용될 변수를 초기화하며 tick()함수의 호출 빈도를 설정합니다. 
AUDPActor::AUDPActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 스폰 시 충돌판정을 하지 않기 위한 변수
	bNoCollisionFail = true;
	// 캐릭터 스폰 정보 미리 정의
	SpawnInfo.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// 캐릭터 생성 시 사용할 Vector와 Rotator 초기화
	myLoc = FVector(-700.0f, 370.0f, 227.0f);
	myRot = FRotator(0, 0, 0);

	// 미리 스폰할 캐릭터 갯수
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

		// 최초 실행 시 CharNum 값에 따라 캐릭터를 생성하고 전부 비활성화시킵니다. (후에 접속한 유저 수에 따라 비활성화된 캐릭터를 활성화시킵니다.)
		for (int32 i = 0; i < CharNum; i++)
		{
			// 스폰한 캐릭터 액터를 WorldCharacter 리스트에 넣습니다. 
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

	// 소켓이 생성되지 않았을 시
	if (!IsHeart)
	{
		// 최초 소켓을 한 번 생성합니다.

		// 하트비트를 송신하는 타임 핸들러
		GetWorldTimerManager().SetTimer(HeartBeatSenderHandle, this, &AUDPActor::HeartBeatSender, 1.0f, true);
		// 하트비트를 수신 여부를 일정 주기마다 초기화시키는 타임 핸들러
		GetWorldTimerManager().SetTimer(HeartBeatReceverHandle, this, &AUDPActor::HeartBeatRecever, 4.2f, true);
		// 하트비트가 일정 주기마다 수신되지 않으면 클라이언트를 종료하는 타임 핸들러
		//GetWorldTimerManager().SetTimer(ConnectManagerHandle, this, &AUDPActor::UnConnectDistroyGame, 7.777f, true);
		IsHeart = true;
	}

	
}

// UDP 액터가 파괴될 때 불려지는 함수
// 서버에 로그아웃을 요청합니다.
void AUDPActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

// Called every frame
void AUDPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// UDP소켓을 생성하는 함수. 
// 소켓 설정이 모두 완료되면 ToServerLocation()을 호출합니다.
void AUDPActor::SetUdpSocket()
{
	UMyGameInstance* myGameInstance = Cast<UMyGameInstance>(GetGameInstance());

	// 송신할 서버의 IP를 정의합니다.
	//FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address ip(52, 78, 90, 90);

	// InternetAddress를 생성합니다.
	addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	// 서버의 IP를 셋팅합니다.
	addr->SetIp(ip.Value);
	// 서버의 포트를 세팅합니다.
	addr->SetPort(800);

	// UDP 소켓을 생성합니다.
	Socket = FUdpSocketBuilder("UDPSocket").AsReusable().WithBroadcast();
	// 보낼 버퍼 사이즈를 정합니다.
	int32 BufferSize = 2 * 1024 * 1024;

	// 보내고 받을 버퍼 사이즈를 정의합니다.
	Socket->SetSendBufferSize(BufferSize, BufferSize);
	Socket->SetReceiveBufferSize(BufferSize, BufferSize);

	// 소켓을 연결한 뒤 bool변수에 결과값을 넣습니다.
	bool connected = Socket->Connect(*addr);

	/*-----------------------------------------------------------------------------------------------------*/

	if (myGameInstance->IsLogin == 0)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> LoginObject = MakeShareable(new FJsonObject);

		// 생성한 JsonObject에 메시지의 고유번호와 
		LoginObject->SetStringField("RequestNum", "1001");
		// 메시지를 송신하는 유저를 추가합니다.
		LoginObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString OutputLogin;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(LoginObject.ToSharedRef(), Writer);

		myGameInstance->IsLogin = 1;

		AESEncryption(OutputLogin);
	}

	// 소켓이 연결되었을 경우
	if (connected)
	{
		// UDP Receiver의 대기시간을 정의합니다.
		FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
		// UDP Receiver를 생성합니다.
		UDPReceiver = new FUdpSocketReceiver(Socket, ThreadWaitTime, TEXT("UDP_RECEIVER"));
		// UDP Receiver과 메시지를 받는 함수를 연동합니다.
		UDPReceiver->OnDataReceived().BindUObject(this, &AUDPActor::RecvFromServer);
		// UDP Receiver를 시작합니다.
		UDPReceiver->Start();

		// 소켓이 생성되고 연결이 수립된 뒤, 리시버가 실행되면 현재 상태를 보냅니다.
		ToServerState(1);
		ToServerState(2);

		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> LoginObject = MakeShareable(new FJsonObject);

		// 생성한 JsonObject에 메시지의 고유번호와 
		LoginObject->SetStringField("RequestNum", "1003");
		// 메시지를 송신하는 유저를 추가합니다.
		LoginObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString OutputLogin;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
		// JsonObject를 직렬화합니다.
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

// 서버로부터 메시지를 받는 함수
void AUDPActor::RecvFromServer(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	// 메시지를 수신할 버퍼의 사이즈를 동적으로 할당합니다.
	RecvData = (char*)malloc(ArrayReaderPtr->Num() + 1);

	// 버퍼의 내용을 복사합니다.
	memcpy(RecvData, ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	RecvData[ArrayReaderPtr->Num()] = 0;
	// 버퍼의 내용을 String에 담습니다.
	RecvString = ANSI_TO_TCHAR(RecvData);

	// 복호화
	FString AESKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	bool IsDecrypted = false;
	RecvString = CBC_AESDecryptData(RecvString, AESKey, "ABCDEF0123456789", IsDecrypted);
	
	if (IsDecrypted)
	{
		// 수신한 String을 JSONObject로 변환합니다.
		TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);
		// Reader를 통해 JSONObject를 읽을 수 있도록 변환합니다.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RecvString);

		// 직렬화된 JSONObject을 원상태로 복구시킵니다.
		if (FJsonSerializer::Deserialize(Reader, RecvJsonObject))
		{
			// 응답에 대한 고유번호를 얻습니다.
			AnswerNum = FCString::Atoi(*RecvJsonObject->GetStringField(TEXT("AnswerNum")));

			// 응답이 ANS_CURRENTUSER(1120) 또는 CHARMOVE(1121)일 경우 RequestUser를 받지 않습니다.
			if (AnswerNum != 1121 && AnswerNum != 1120)
			{
				RequestUser = *RecvJsonObject->GetStringField(TEXT("RequestUser"));
			}

			// 응답에 따라 다른 함수를 호출합니다.
			switch (AnswerNum)
			{
			case 1000: // HEART_BEAT 하트비트
				Recv_HeartBeat();
				break;

			case 1120: //ANS_CURRENTUSER 현재 캐릭터 목록 응답
				Recv_CurrentUser(RecvJsonObject);
				break;

			case 1171: // CHARSTATE 캐릭터 상태 응답
				Recv_CharState(RequestUser, RecvJsonObject);
				break;

			case 2001: //YES LOGON 로그인 응답
				Recv_LogIn();
				break;

			case 2002: //YES LOGOUT 로그아웃 응답
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

		// 동적으로 할당한 버퍼를 해제합니다.
		free(RecvData);
	}
	else
	{
		free(RecvData);
	}

}

// 하트비트를 송신하는 함수
void AUDPActor::HeartBeatSender()
{
	if (!IsSocket)
	{
		SetUdpSocket();
		IsSocket = true;
	}

	if (IsSocket)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "1000");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString HeartBeat;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&HeartBeat);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(HeartBeat);
	}

}

// 하트비트 수신 여부를 일정 주기마다 초기화시키는 함수
void AUDPActor::HeartBeatRecever()
{
	IsConnect = false;
}

// 하트비트가 일정 주기마다 수신되지 않으면 클라이언트를 종료하는 함수
void AUDPActor::UnConnectDistroyGame()
{
	if (!IsConnect)
	{
		Dialog.Open(EAppMsgType::Ok, FText::FromString(TEXT("Network connection Fail.")));
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}
}

// 하트비트를 수신하는 함수
void AUDPActor::Recv_HeartBeat()
{
	IsConnect = true;
}

// 서버로부터 현재 접속한 유저 목록을 받아 캐릭터의 활성화 & 비활성화 여부를 결정하는 함수
void AUDPActor::Recv_CurrentUser(TSharedPtr<FJsonObject> RecvJsonObject)
{
	if (GEngine)
	{
		// 이미 활성화되어있던 유저 목록을 담는 Array
		TArray<FString> LastPlayerNameArray = PlayerNameArray;
		// 새로운 캐릭터를 할당받아야 하는 유저 목록을 담는 Array
		TArray<FString> NewPlayerNameArray;
		// 할당된 캐릭터를 해제해야 하는 유저 목록을 담는 Array
		TArray<FString> DeletePlayerNameArray;

		// 현재 접속한 유저들의 닉네임을 관리하는 Array. 이 배열의 인덱스 갯수에 따라 캐릭터를 활성화시킵니다.
		// Array를 초기화합니다.
		PlayerNameArray.Empty();

		// 서버로부터 PlayerList 배열을 받습니다. {"PlayerList":["941", "941", "941", "941", "941", "941", "941"],"AnswerNum":1120}
		TArray<TSharedPtr<FJsonValue>> objArray = RecvJsonObject->GetArrayField(TEXT("PlayerList"));

		// PlayerNameArray에 자신을 제외한 나머지 모든 유저들을 추가합니다.
		for (int32 i = 0; i < objArray.Num(); i++)
		{
			FString playerName = objArray[i]->AsString();
			if (!playerName.Equals(PlayerID))
			{
				PlayerNameArray.Add(playerName);
			}
		}

		// PlayerNameArray의 배열 수 만큼 반복문 돌려서 이전에 캐릭터가 활성화된 유저였는지 확인
		for (int32 m = 0; m < PlayerNameArray.Num(); m++)
		{
			if (!LastPlayerNameArray.Contains(PlayerNameArray[m]))
			{
				NewPlayerNameArray.Add(PlayerNameArray[m]);
			}
		}


		// 캐릭터가 활성화되지 않은 유저라면 새 캐릭터 할당
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

		// LastPlayerNameArray의 배열 수 만큼 반복문 돌려서 캐릭터가 활성화된 유저가 현재도 남아있는지 확인
		for (int32 p = 0; p < LastPlayerNameArray.Num(); p++)
		{
			if (!PlayerNameArray.Contains(LastPlayerNameArray[p]))
			{
				DeletePlayerNameArray.Add(LastPlayerNameArray[p]);
			}
		}

		// 사라진 유저의 캐릭터는 비활성화 시킴
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

// 캐릭터의 상태를 송신하는 함수
void AUDPActor::ToServerState(int32 stateNum)
{
	if (GetWorld())
	{
		// 상태값을 넣을 JsonObject를 정의합니다.
		TSharedPtr<FJsonObject> StateObject = MakeShareable(new FJsonObject);

		// 생성한 JsonObject에 메시지의 고유번호와 
		StateObject->SetStringField("RequestNum", "1071");

		// 메시지를 송신하는 유저를 추가합니다.
		StateObject->SetStringField("RequestUser", PlayerID);

		// JsonObject에 상태값을 추가합니다.
		StateObject->SetStringField("StateNum", FString::FromInt(stateNum));

		if (stateNum == 1)
		{
			// 월드에서 캐릭터의 컨트롤러를 받습니다.
			myCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld()->GetFirstPlayerController(), 0);

			if (myCharacter)
			{
				// Vector에 내 캐릭터의 위치를 받습니다.
				myLocation = myCharacter->GetActorLocation();

				// Rotator에 내 캐릭터의 위치를 받습니다.
				myRotation = myCharacter->GetActorRotation();

				// 각 변수에 Vector의 X,Y,Z 값을 분할합니다.
				myLocationX = FString::SanitizeFloat(myLocation.X);
				myLocationY = FString::SanitizeFloat(myLocation.Y);
				myLocationZ = FString::SanitizeFloat(myLocation.Z);
				myRotationYaw = FString::SanitizeFloat(myRotation.Yaw);

				// JsonObject에 좌표들을 추가합니다.
				StateObject->SetStringField("LocationX", myLocationX);
				StateObject->SetStringField("LocationY", myLocationY);
				StateObject->SetStringField("LocationZ", myLocationZ);

				// JsonObject에 회전값을 추가합니다.
				StateObject->SetStringField("RotationYaw", myRotationYaw);
			}
		}

		// JsonObject를 String으로 변환할 자료형
		FString OutputState;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputState);
		// JsonObject를 직렬화합니다.
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

// 로그인을 수신하는 함수
void AUDPActor::Recv_LogIn()
{
}

// 로그아웃을 수신하는 함수
void AUDPActor::Recv_LogOut()
{
}

void AUDPActor::SessionLogout()
{
	// 서버로 전송할 JsonObject를 생성합니다.
	TSharedPtr<FJsonObject> LogoutObject = MakeShareable(new FJsonObject);

	// 생성한 JsonObject에 메시지의 고유번호와 
	LogoutObject->SetStringField("RequestNum", "1002");
	// 메시지를 송신하는 유저를 추가합니다.
	LogoutObject->SetStringField("RequestUser", PlayerID);

	// JsonObject를 String으로 변환할 자료형
	FString OutputLogin;
	// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
	TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&OutputLogin);
	// JsonObject를 직렬화합니다.
	FJsonSerializer::Serialize(LogoutObject.ToSharedRef(), Writer);

	AESEncryption(OutputLogin);
}

// 서버로부터 상태 메시지가 왔을 때 처리하는 함수
void AUDPActor::Recv_CharState(FString requestUser, TSharedPtr<FJsonObject> RecvJsonObject)
{
	//Server sending response :{"StateNum":"1","AnswerNum":1171,"RequestUser":"8432"}
	// PlayerName은 PlayerObj의 키를 저장합니다.

	FString PlayerName = requestUser;

	TSharedPtr<FJsonObject> PlayerStateObj = RecvJsonObject;

	int32 StateNum = FCString::Atoi(*PlayerStateObj->GetStringField("StateNum"));

	if (StateNum == 1)
	{
		// PlayerLocationobj에서 X,Y,Z 좌표와 Y축 회전값을 얻습니다.
		UPROPERTY(EditAnywhere)
		float playerX;
		UPROPERTY(EditAnywhere)
		float playerY;
		UPROPERTY(EditAnywhere)
		float playerZ;
		UPROPERTY(EditAnywhere)
		float PlayerYaw;

		// playerX, playerY, playerZ의 세 float값을 하나의 Vector 단위로 묶기 위한 자료형
		FVector playerVector;

		// 회전값을 하나로 묶기 위한 자료형
		FRotator playerRotator;

		if (GEngine)
		{
			// RecvMessage : 
			//{"AnswerNum":1121,"PlayerLocation":{"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"},"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}}}

			// Playerobj에 PlayerLocation의 값을 가져옵니다.
			TSharedPtr<FJsonObject> Playerobj = RecvJsonObject->GetObjectField(TEXT("PlayerLocation"));

			//({"941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"},"942":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}})

			// PlayerNameArray의 값 만큼 반복문 실행을 하면 안되고...
			// PlayerObject 값만큼 반복문 실행한 다음에 내 아이디 오면 제껴야됨

			for (int32 i = 0; i < PlayerNameArray.Num(); i++)
			{
				FString PlayerName = PlayerNameArray[i];

				// PlayerLocationObj은 Playerobj에서 PlayerName에 따라 LocationX, LocationY, LocationZ 단위로 JSONObject의 객체를 가져옵니다. 
				//("LocationX" : "0.0")

				if (!PlayerName.Equals(PlayerID))
				{
					PlayerStateObj = Playerobj->GetObjectField(PlayerName);
					// "941":{"LocationY":"491.2398","LocationZ":"228.80763","LocationX":"-770.00256"}

					if (PlayerStateObj.GetSharedReferenceCount() > 0)
					{
						// PlayerLocationobj에서 X,Y,Z 좌표를 얻습니다.
						playerX = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationX")));
						playerY = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationY")));
						playerZ = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("LocationZ")));

						// playerX, playerY, playerZ의 세 float값을 하나의 Vector 단위로 묶기 위한 자료형
						playerVector = FVector(playerX, playerY, playerZ);

						// PlayerLocationobj에서 회전값을 얻습니다.
						//playerPitch = FCString::Atof(*PlayerLocationobj->GetStringField(TEXT("RotationPitch")));
						//playerRoll = FCString::Atof(*PlayerLocationobj->GetStringField(TEXT("RotationRoll")));
						PlayerYaw = FCString::Atof(*PlayerStateObj->GetStringField(TEXT("RotationYaw")));

						// 회전값을 하나로 묶기 위한 자료형
						playerRotator = FRotator(0.0f, PlayerYaw, 0.0f);

						// 미리 생성된 캐릭터 액터의 수 만큼 반복문 실행
						for (int32 j = 0; j < WorldCharacter.Num(); j++)
						{
							// WorldCharacter[i]의 닉네임이 할당되어있지 않다면 반복문 종료
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
		// 미리 생성된 캐릭터 액터의 수 만큼 반복문 실행
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

// 몬스터 반경과 충돌했을 때 몬스터 활성화를 요청하는 함수
void AUDPActor::RequestMonsterBegin()
{
	if (IsSocket)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "3001");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString MosterReqStr;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

// 몬스터 공격 성공 시 몬스터 공격 판정을 요청하는 함수
void AUDPActor::RequestMonsterAttack(float damage)
{
	if (IsSocket)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "3011");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// 데미지를 추가합니다.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject를 String으로 변환할 자료형
		FString MosterReqStr;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

// 몬스터 공격 성공 시 몬스터 공격 판정을 요청하는 함수
void AUDPActor::RequestMonsterAttack2(float damage)
{
	if (IsSocket)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "3012");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// 데미지를 추가합니다.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject를 String으로 변환할 자료형
		FString MosterReqStr;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);

		AESEncryption(MosterReqStr);
	}
}

void AUDPActor::RequestUserAttack(float damage)
{
	if (IsSocket)
	{
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "3021");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);
		// 데미지를 추가합니다.
		HeartBeatObject->SetStringField("RequestDamage", FString::SanitizeFloat(damage));

		// JsonObject를 String으로 변환할 자료형
		FString MosterReqStr;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject를 직렬화합니다.
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
		// 서버로 전송할 JsonObject를 생성합니다.
		TSharedPtr<FJsonObject> HeartBeatObject = MakeShareable(new FJsonObject);
		// 생성한 JsonObject에 메시지의 고유번호와 
		HeartBeatObject->SetStringField("RequestNum", "3031");
		// 메시지를 송신하는 유저를 추가합니다.
		HeartBeatObject->SetStringField("RequestUser", PlayerID);

		// JsonObject를 String으로 변환할 자료형
		FString MosterReqStr;
		// JsonWriter을 통하여 JsonObject를 String으로 변환합니다.
		TSharedRef<TJsonWriter< >> Writer = TJsonWriterFactory<>::Create(&MosterReqStr);
		// JsonObject를 직렬화합니다.
		FJsonSerializer::Serialize(HeartBeatObject.ToSharedRef(), Writer);
		
		AESEncryption(MosterReqStr);
	}
}

void AUDPActor::AESEncryption(FString requestStr)
{
	FString AESKey = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	// 키를 UTF8로 인코딩
	TCHAR *KeyTChar = AESKey.GetCharArray().GetData();
	FTCHARToUTF8 StringtoUTF8_0(*AESKey);

	FString RequestStr = requestStr;
	// 보낼 Byte의 크기를 담기 위한 변수
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