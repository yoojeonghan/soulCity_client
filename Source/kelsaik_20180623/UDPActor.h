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

	// UDP ���Ͱ� �ı��� �� �ҷ����� �Լ�
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/*------------------------------------------------------------------------------*/

	FMessageDialog Dialog;

	/*------------------------------------------------------------------------------*/

	// ��Ʈ��Ʈ�� �ֱ������� �۽��ϱ� ���� Ÿ�̸� �ڵ�
	FTimerHandle HeartBeatSenderHandle;
	// ��Ʈ��Ʈ�� �ֱ������� �����ϱ� ���� Ÿ�̸� �ڵ�
	FTimerHandle HeartBeatReceverHandle;
	// ��Ʈ��Ʈ�� ���� �⵿ֱ�� ���� ���� ��� ������ �����ϱ� ���� Ÿ�̸� �ڵ�
	FTimerHandle ConnectManagerHandle;

	/*------------------------------------------------------------------------------*/

	// InternetAddress�� �����ϱ� ���� ������
	TSharedPtr<FInternetAddr> addr;
	// UDP ����
	FSocket* Socket;
	// UDP ���Ͽ� �߰��� ���ù�
	FUdpSocketReceiver* UDPReceiver = nullptr;
	// UDP ���� ����
	char* RecvData;
	// ���ŵ� ���۸� String���� ��ȯ�մϴ�.
	FString RecvString;
	// ���ŵ� �޽����� ������ȣ�� ����ϴ�.
	int32 AnswerNum;
	// �޽����� �߽��ڸ� ����ϴ�. 
	FString RequestUser;

	Akelsaik_20180623GameMode* MyGameMode;

	/*------------------------------------------------------------------------------*/

	// ������ �����Ǿ����� Ȯ���ϱ� ���� bool��. ���� ���� �� true�� �ٲ�ϴ�.
	UPROPERTY(EditAnywhere)
	bool IsHeart = false;

	// ���� ���� �� ������ Ȯ���ϱ� ���� bool��. ������ ���ʷ� �����Ǹ� true�� �ٲ�ϴ�.
	UPROPERTY(EditAnywhere)
	bool IsConnect = false;

	UPROPERTY(EditAnywhere)
	bool IsSocket = false;

	/*------------------------------------------------------------------------------*/

	// ���� ������ �������� �г����� �����ϴ� Array. �� �迭�� ���ڿ� ���� ĳ���͸� Ȱ��ȭ��ŵ�ϴ�.
	UPROPERTY(EditAnywhere)
	TArray<FString> PlayerNameArray;

	// ���� ������ �������� ĳ���� ���͸� �����ϴ� Array.
	UPROPERTY(EditAnywhere)
	TArray<class AWorldCharacter*> WorldCharacter;

	AMonsterCharacter* MonsterCharacter;

	UPROPERTY()
	class AItemActor* ItemActor;
	/*------------------------------------------------------------------------------*/

	// ���� ���� ���� ��� ĳ���͸� ����� ����⿡�� �ڿ����� ���ϹǷ�, ���� Ǯ ���� �������� ����ϱ� ���� �����Դϴ�.
	// ���� ���� �� �� ���� ���� ĳ���͸� �����ϰ� ���� ��Ȱ��ȭ��ŵ�ϴ�. 
	// �Ŀ� ������ ���� ���� ���� ��Ȱ��ȭ�� ĳ���͸� Ȱ��ȭ��ŵ�ϴ�.
	int32 CharNum;

	// ������ ĳ������ ���� ��ġ�� �۽��ϱ� ���� �ڽ��� ĳ���� ���͸� �������մϴ�.
	ACharacter* myCharacter;

	/*------------------------------------------------------------------------------*/

public:

	// UDP������ �����ϴ� �Լ�
	void SetUdpSocket();
	// ��ġ ���� - ĳ������ ��ġ�� �۽��ϴ� �Լ�
	//void ToServerLocation();
	// �ִϸ��̼� ���� - ĳ������ ���¸� �۽��ϴ� �Լ�
	void ToServerState(int32 stateNum);

	/*------------------------------------------------------------------------------*/

	// �����κ��� �޽����� �޴� �Լ�
	void RecvFromServer(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	/*------------------------------------------------------------------------------*/

	// �α����� �����ϴ� �Լ�
	void Recv_LogIn();
	// �α׾ƿ��� �����ϴ� �Լ�
	void Recv_LogOut();
	// �����κ��� ���� ������ ���� ����� �޾� ĳ������ Ȱ��ȭ & ��Ȱ��ȭ ���θ� �����ϴ� �Լ�
	void Recv_CurrentUser(TSharedPtr<FJsonObject> RecvJsonObject);
	// �����κ��� ���� �޽����� ���� �� ó���ϴ� �Լ�
	void Recv_CharState(FString requestUser, TSharedPtr<FJsonObject> RecvJsonObject);

	/*------------------------------------------------------------------------------*/

	// ��Ʈ��Ʈ�� �۽��ϴ� �Լ�
	void HeartBeatSender();
	// ��Ʈ��Ʈ�� �����ϴ� �Լ�
	void Recv_HeartBeat();
	// ��Ʈ��Ʈ ���� ���θ� ���� �ֱ⸶�� �ʱ�ȭ��Ű�� �Լ�
	void HeartBeatRecever();
	// ��Ʈ��Ʈ�� ���� �ֱ⸶�� ���ŵ��� ������ Ŭ���̾�Ʈ�� �����ϴ� �Լ�
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

	// ĳ������ ���� ��ǥ�� �ҷ����� ���� Vector
	FVector myLocation;
	FRotator myRotation;

	// ĳ������ ���� ��ǥ�� String���� ��ȯ�ϱ� ���� �ڷ���
	FString myLocationX;
	FString myLocationY;
	FString myLocationZ;

	// ĳ������ ���� ������ String���� ��ȯ�ϱ� ���� �ڷ���
	FString myRotationPitch;
	FString myRotationRoll;
	FString myRotationYaw;

	/*------------------------------------------------------------------------------*/

	// ���� ���� �� ������ �̸� ����
	FActorSpawnParameters SpawnInfo;
	// ĳ���� ���� �� �浹������ ���� �ʱ� ���� ����
	bool bNoCollisionFail;

	// ĳ���� ���� �� ����� Vector�� Rotator
	FVector myLoc;
	FRotator myRot;

	/*------------------------------------------------------------------------------*/

	// �α���, ȸ������ �ý��� ���� �� �ӽ÷� Ŭ���̾�Ʈ�� �����ϱ� ���� ���� Ű
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
