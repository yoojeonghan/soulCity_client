// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
#include "kelsaik_20180623GameMode.h"
#include "ItemActor.h"
#include "EngineUtils.h"
#include <Runtime/Engine/Classes/Engine/TextRenderActor.h>
#include <Runtime/Engine/Classes/Animation/BlendSpace1D.h>
#include "kelsaik_20180623GameMode.h"
/*-----------------------------------------------*/
#include "MonsterCharacter.generated.h"

UCLASS()
class KELSAIK_20180623_API AMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();

protected:

	virtual void PostInitializeComponents() override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	int32 StateNum;

	USkeletalMeshComponent * CharacterMeshComponet;

	UAnimationAsset* IdleAnimAsset;
	UAnimationAsset* ActiveIdleAnimAsset;
	UAnimationAsset* JogAnimAsset;
	UAnimationAsset* StartAnimAsset;
	UAnimationAsset* Attack1Montage;
	UAnimationAsset* DieAnimAsset;
	UAnimationAsset* NuckbackAsset;

	AItemActor* ItemActor;
	//AItemActor* ItemActor;

	// declare mesh component
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MyMesh;

	float SphereRadius;

	UPROPERTY(EditAnywhere)
	UBoxComponent* AttackBox;

	FVector destinationLocation;
	
	FVector CurrentLocation;
	FRotator CurrentRotator;

	bool IsUpdateState;

	float speed;

	bool IsJump;

	bool IsMoving;

	bool IsAttack;

	bool IsJog;

	bool IsActive;

	bool IsActiveUI;

	UPROPERTY(BlueprintReadWrite)
	bool WasAttackDamage;

	UPROPERTY(BlueprintReadWrite)
	bool IsAttackBound;

	FTimerHandle IdleStateTimerHandle;
	FTimerHandle JogFalseTimerHandle;
	FTimerDelegate TimerDel;
	FTimerHandle JogToIdleimerHandle;
	USceneComponent* RootComponent;

	Akelsaik_20180623GameMode* MyGameMode;

	UPROPERTY()
	class Akelsaik_20180623Character* MyCharacter;
	
	UPROPERTY()
	class AUDPActor* UDPActor;

	FHttpModule* Http;

	FString PlayerID;

public:

	UFUNCTION(BlueprintCallable)
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION(BlueprintCallable)
	void EndOverlap(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 캐릭터의 상태를 변환하기 위한 함수
	UFUNCTION()
	void FUpdateState(int32 stateNum);

	void FJogFalse();

	UFUNCTION()
	void FJogStateStop();

	void FJogToIdle();

	UFUNCTION(BlueprintCallable)
	void AttackBound();

	void MonsterDie();

	void MonsterMove(FVector monsterVector, FRotator monsterRotator);

	void MonsterMove(FRotator monsterRotator);

	void MyHttpCall();

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
