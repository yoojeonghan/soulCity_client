﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UDPActor.h"
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
#include "EditableTextBox.h"
#include "Blueprint/UserWidget.h"
#include "TextBlock.h"
#include "kelsaik_20180623.h"
#include "kelsaik_20180623GameMode.h"

/*--------------------------------------------------*/
#include "TCPActor.generated.h"

UCLASS()
class KELSAIK_20180623_API ATCPActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATCPActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// TCP 액터가 파괴될 때 불려지는 함수
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/*------------------------------------------------------------------------------*/

	// 로그인, 회원가입 시스템 구현 전 임시로 클라이언트를 구분하기 위한 랜덤 키
	FString CharID;

	/*------------------------------------------------------------------------------*/

	// InternetAddress를 저장하기 위한 포인터
	TSharedPtr<FInternetAddr> addr;
	// TCP 소켓
	FSocket* Socket;
	// TCP 연결 소켓
	FSocket* ConnectionSocket;

	FIPv4Endpoint RemotAddressForConnection;
	// TCP 수신 버퍼
	char* RecvData;
	// 수신된 버퍼를 String으로 변환합니다.
	FString RecvString;
	// 수신된 메시지의 고유번호를 얻습니다.
	int32 AnswerNum;
	// 메시지의 발신자를 얻습니다. 
	FString RequestUser;
	// 메시지의 내용을 얻습니다.
	FString AnswerString;
	// TCP 수신 타이머 핸들
	FTimerHandle TCPReceiverHandle;

	/*------------------------------------------------------------------------------*/

public:
	// TCP 소켓을 생성하는 함수
	void SetTCPSocket();
	//
	void SetTCPConnection();

	void TCPSocketListener();

	FString StringFromBinaryArray(TArray<uint8>& BinaryArray);
	/*------------------------------------------------------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void SendingChat(FText sendtext);

	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void RecevingChat(FString receivedString);

	void SessionLogout();

	/*------------------------------------------------------------------------------*/

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UUserWidget* CurrentWidget;
};
