// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SharedPointer.h"
#include "Engine.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "EditableTextBox.h"
#include "EditableText.h"
#include "TextBlock.h"
#include "VerticalBox.h"
#include "ScrollBox.h"
#include "FileHelper.h"
#include "Paths.h"
#include "ProgressBar.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Overlay.h"

/*------------------------------------------------------------------------------*/
#include "kelsaik_20180623GameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyBindableEvent);

UCLASS(minimalapi)
class Akelsaik_20180623GameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	Akelsaik_20180623GameMode();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerWidget", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY()
	class UUserWidget* CurrentWidget;

	UPROPERTY()
	class ATCPActor* TCPActor;

	UPROPERTY()
	class AMatchingActor* MatchingActor;

	UPROPERTY()
	class AUDPActor* UDPActor;

	UPROPERTY()
	class Akelsaik_20180623Character* MyCharacter;

public:
	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void FSendChatting();

	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void FReceivedChatting(FString Recvingtext, FString Sender);

	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void FLoginUser(FString LoginUser);

	UFUNCTION(BlueprintCallable, Category = "Chatting")
	void FLogoutUser(FString LoginUser);

	UFUNCTION(BlueprintCallable, Category = "Exit")
	void FGameExit();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void FAttack1();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void FAttack2();

	UFUNCTION(BlueprintCallable, Category = "Matching")
	void FRequestMatching();

	UFUNCTION(BlueprintCallable, Category = "Matching")
	void FRequestMatchingCansel();

	UFUNCTION(BlueprintCallable, Category = "Matching")
	void RecevingMatch();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void RequestMonsterBegin();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void UpdateMonsterHPBar(float hP);

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void UpdateMyHPBar(float hP);

	//UPROPERTY(BlueprintAssignable)
	//FMyBindableEvent ChangeMap;
	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void VisibleMonsterUI(bool isVisible);

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void CreateTeamUI(TArray<FString> playerNameArray);

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void UpdateUserHPBar(FString userID, float hP);

	//	FFileHelper::SaveStringToFile(FString::FromInt(RandCharNum), *(FPaths::ProjectDir()+FString("GameSession")));

	UFUNCTION(BlueprintCallable, Category = "Login")
	void UserLogin();

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void ShowInventory();

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	int32 GetItemNum();

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	TArray<FString> GetRanking();

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void ShowRanking();

	void OnResponseReceived2(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "UserUI")
	void CreateRankStringObject();

public:
	// 로그인, 회원가입 시스템 구현 전 임시로 클라이언트를 구분하기 위한 랜덤 키
	//int32 RandCharNum;

	TArray<FString> PlayerNameArray;

	TArray<UProgressBar*> ProgressArray;

	TArray<FString> RankingArray;

	FHttpModule* Http;

	FString CharID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ItemNum;
};