// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Engine.h"
#include "SocketSubsystem.h"
#include "SharedPointer.h"
#include "IPv4Address.h"
#include "Json.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "UnrealMathUtility.h"
#include "JSONObjectConverter.h"
#include "WorldCharacter.h"
#include "MonsterCharacter.h"
#include "MyGameInstance.h"
#include "ItemActor.h"
//#include "AESBPLibrary.h"
#include "kelsaik_20180623GameMode.h"

/*--------------------------------------------------*/

#include "UDPActor.generated.h"

UCLASS()
class KELSAIK_20180623_API AUDPActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUDPActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// UDP 액터가 파괴될 때 불려지는 함수
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/*------------------------------------------------------------------------------*/

	FMessageDialog Dialog;

	/*------------------------------------------------------------------------------*/

	// 하트비트를 주기적으로 송신하기 위한 타이머 핸들
	FTimerHandle HeartBeatSenderHandle;
	// 하트비트를 주기적으로 수신하기 위한 타이머 핸들
	FTimerHandle HeartBeatReceverHandle;
	// 하트비트가 일정 주기동안 오지 않을 경우 게임을 종료하기 위한 타이머 핸들
	FTimerHandle ConnectManagerHandle;

	/*------------------------------------------------------------------------------*/

	// InternetAddress를 저장하기 위한 포인터
	TSharedPtr<FInternetAddr> addr;
	// UDP 소켓
	FSocket* Socket;
	// UDP 소켓에 추가될 리시버
	FUdpSocketReceiver* UDPReceiver = nullptr;
	// UDP 수신 버퍼
	char* RecvData;
	// 수신된 버퍼를 String으로 변환합니다.
	FString RecvString;
	// 수신된 메시지의 고유번호를 얻습니다.
	int32 AnswerNum;
	// 메시지의 발신자를 얻습니다. 
	FString RequestUser;

	Akelsaik_20180623GameMode* MyGameMode;

	/*------------------------------------------------------------------------------*/

	// 소켓이 생성되었는지 확인하기 위한 bool값. 소켓 생성 시 true로 바뀝니다.
	UPROPERTY(EditAnywhere)
	bool IsHeart = false;

	// 소켓 생성 후 연결을 확인하기 위한 bool값. 연결이 최초로 성립되면 true로 바뀝니다.
	UPROPERTY(EditAnywhere)
	bool IsConnect = false;

	UPROPERTY(EditAnywhere)
	bool IsSocket = false;

	/*------------------------------------------------------------------------------*/

	// 현재 접속한 유저들의 닉네임을 관리하는 Array. 이 배열의 숫자에 따라 캐릭터를 활성화시킵니다.
	UPROPERTY(EditAnywhere)
	TArray<FString> PlayerNameArray;

	// 현재 접속한 유저들의 캐릭터 액터를 관리하는 Array.
	UPROPERTY(EditAnywhere)
	TArray<class AWorldCharacter*> WorldCharacter;

	AMonsterCharacter* MonsterCharacter;

	UPROPERTY()
	class AItemActor* ItemActor;
	/*------------------------------------------------------------------------------*/

	// 유저 수에 따라 계속 캐릭터를 만들고 지우기에는 자원낭비가 심하므로, 액터 풀 같은 개념으로 사용하기 위한 변수입니다.
	// 최초 실행 시 이 값에 따라 캐릭터를 생성하고 전부 비활성화시킵니다. 
	// 후에 접속한 유저 수에 따라 비활성화된 캐릭터를 활성화시킵니다.
	int32 CharNum;

	// 서버로 캐릭터의 현재 위치를 송신하기 위해 자신의 캐릭터 액터를 재정의합니다.
	ACharacter* myCharacter;

	/*------------------------------------------------------------------------------*/

public:

	// UDP소켓을 생성하는 함수
	void SetUdpSocket();
	// 위치 연동 - 캐릭터의 위치를 송신하는 함수
	//void ToServerLocation();
	// 애니메이션 연동 - 캐릭터의 상태를 송신하는 함수
	void ToServerState(int32 stateNum);

	/*------------------------------------------------------------------------------*/

	// 서버로부터 메시지를 받는 함수
	void RecvFromServer(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	/*------------------------------------------------------------------------------*/

	// 로그인을 수신하는 함수
	void Recv_LogIn();
	// 로그아웃을 수신하는 함수
	void Recv_LogOut();
	// 서버로부터 현재 접속한 유저 목록을 받아 캐릭터의 활성화 & 비활성화 여부를 결정하는 함수
	void Recv_CurrentUser(TSharedPtr<FJsonObject> RecvJsonObject);
	// 서버로부터 상태 메시지가 왔을 때 처리하는 함수
	void Recv_CharState(FString requestUser, TSharedPtr<FJsonObject> RecvJsonObject);

	/*------------------------------------------------------------------------------*/

	// 하트비트를 송신하는 함수
	void HeartBeatSender();
	// 하트비트를 수신하는 함수
	void Recv_HeartBeat();
	// 하트비트 수신 여부를 일정 주기마다 초기화시키는 함수
	void HeartBeatRecever();
	// 하트비트가 일정 주기마다 수신되지 않으면 클라이언트를 종료하는 함수
	void UnConnectDistroyGame();

	/*------------------------------------------------------------------------------*/

	void SessionLogout();

	void RequestMonsterBegin();

	void RequestMonsterAttack(float damage);

	void RequestMonsterAttack2(float damage);

	void RequestUserAttack(float damage);

	void Recv_MonsterState(int32 monsterStateNum, TSharedPtr<FJsonObject> recvJsonObject);

	void SendItemLooting();

	void AESEncryption(FString requestStr);

public:

	/*------------------------------------------------------------------------------*/

	// 캐릭터의 현재 좌표를 불러오기 위한 Vector
	FVector myLocation;
	FRotator myRotation;

	// 캐릭터의 현재 좌표를 String으로 변환하기 위한 자료형
	FString myLocationX;
	FString myLocationY;
	FString myLocationZ;

	// 캐릭터의 현재 각도를 String으로 변환하기 위한 자료형
	FString myRotationPitch;
	FString myRotationRoll;
	FString myRotationYaw;

	/*------------------------------------------------------------------------------*/

	// 액터 스폰 시 정보를 미리 정의
	FActorSpawnParameters SpawnInfo;
	// 캐릭터 스폰 시 충돌판정을 하지 않기 위한 변수
	bool bNoCollisionFail;

	// 캐릭터 생성 시 사용할 Vector와 Rotator
	FVector myLoc;
	FRotator myRot;

	/*------------------------------------------------------------------------------*/

	// 로그인, 회원가입 시스템 구현 전 임시로 클라이언트를 구분하기 위한 랜덤 키
	FString PlayerID;
	int32 MonsterStateNum;
	
	bool IsTeamUI;

	/*------------------------------------------------------------------------------*/
	
	static FString ECB_AESEncryptData(FString aes_content, FString aes_key);
	static FString ECB_AESDecryptData(FString aes_content, FString aes_key, bool & result);

	static FString CBC_AESEncryptData(FString aes_content, FString aes_key, FString aes_IV);
	static FString CBC_AESDecryptData(FString aes_content, FString aes_key, FString aes_IV, bool & result);

	/*------------------------------------------------------------------------------*/

	//char *aes_encode(FString sourcestr, FString aes_key, FString aes_IV);
	//char *aes_decode(const char *crypttext, char *key = "");
	//char *base64_encode(const char *data, int data_len);
	//char *base64_decode(const char *data, int data_len, int &out_len);
};
