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
#include "Runtime/UMG/Public/Blueprint/WidgetLayoutLibrary.h"
#include "kelsaik_20180623GameMode.h"

/*---------------------------------------*/

#include "MatchingActor.generated.h"

UCLASS()
class KELSAIK_20180623_API AMatchingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMatchingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// TCP ���Ͱ� �ı��� �� �ҷ����� �Լ�
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/*------------------------------------------------------------------------------*/

	// �α���, ȸ������ �ý��� ���� �� �ӽ÷� Ŭ���̾�Ʈ�� �����ϱ� ���� ���� Ű
	FString PlayerID;

	// ���� ���ȣ
	int32 RoomNumber;
	
	Akelsaik_20180623GameMode* MyGameMode;
	/*------------------------------------------------------------------------------*/

	// InternetAddress�� �����ϱ� ���� ������
	TSharedPtr<FInternetAddr> addr;
	// TCP ����
	FSocket* Socket;
	// TCP ���� ����
	FSocket* ConnectionSocket;

	FIPv4Endpoint RemotAddressForConnection;
	// TCP ���� ����
	char* RecvData;
	// ���ŵ� ���۸� String���� ��ȯ�մϴ�.
	FString RecvString;
	// ���ŵ� �޽����� ������ȣ�� ����ϴ�.
	int32 AnswerNum;
	// �޽����� �߽��ڸ� ����ϴ�. 
	FString RequestUser;
	// �޽����� ������ ����ϴ�.
	FString AnswerString;
	// TCP ���� Ÿ�̸� �ڵ�
	FTimerHandle TCPReceiverHandle;

	FTimerHandle BeginMapTimerHandel;
	FTimerDelegate TimerDel;

	/*------------------------------------------------------------------------------*/

public:
	// TCP ������ �����ϴ� �Լ�
	void SetTCPSocket();
	//
	void SetTCPConnection();

	void TCPSocketListener();

	FString StringFromBinaryArray(TArray<uint8>& BinaryArray);

	/*------------------------------------------------------------------------------*/

	// declare collision component
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* MyCollisionSphere;

	// declare mesh component
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MyMesh;

	float SphereRadius;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION(BlueprintCallable)
	void RequestMatching();

	UFUNCTION(BlueprintCallable)
	void CanselMatching();
	
	UFUNCTION()
	void RecevingMatch();

	
};
