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
#include "EngineUtils.h"
#include <Runtime/Engine/Classes/Engine/TextRenderActor.h>
#include <Runtime/Engine/Classes/Animation/BlendSpace1D.h>

/*-----------------------------------------------*/

#include "WorldCharacter.generated.h"

UCLASS()
class KELSAIK_20180623_API AWorldCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWorldCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// WorldCharacter 액터에 적용될 SkeletalMeshComponent 정의
	USkeletalMeshComponent * CharacterMeshComponet;

	// WorldCharacter 액터에 적용될 MovementComponent 정의
	UCharacterMovementComponent* CharacterMovementComponet;

	//	APlayerController *CharacterController;

	// WorldCharacter 액터의 아이디 정의. 구분값 및 머리위에 표시될 TextRenderComponent와 연계됨
	FString CharID;

	// 각 캐릭터마다 머리 위에 띄워질 TextReanderComponent 정의
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CharIDText;

	// 캐릭터 상태 정의
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CharStateNum;

	// 캐릭터의 좌표를 받기 위한 Vector 자료형
	FVector PlayerVector;

	// 캐릭터의 회전값을 받기 위한 Rotator 자료형
	FRotator PlayerRotator;

	// TextRenderCompoent를 한 번만 적용하기 위해 사용될 bool 변수
	bool IsIDText;

	bool IsWorld;

	bool IsBegining;

	bool IsJog;

	// 애니메이션 에셋

	UAnimationAsset* IdleAnimAsset;
	UAnimationAsset* JogAnimAsset;
	UAnimationAsset* StartAnimAsset;
	UAnimationAsset* ChangeToIdleFromJogAsset;
	UAnimationAsset* JumpAnimAsset_JumpStart;
	UAnimationAsset* JumpAnimAsset_Jumping;
	UAnimationAsset* JumpAnimAsset_JumpLand;
	UAnimationAsset* JumpAnimAsset_JumpEnd;
	UAnimMontage* JumpMontage;
	UAnimMontage* Attack1Montage;
	UAnimationAsset* Attack2Montage;

	FVector destinationLocation;
	FVector CurrentLocation;


	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* AttackBox2;

	bool IsUpdateState;

	float speed;

	bool IsJump;

	bool IsMoving;

	bool IsAttack;

	bool IsAttack2;

	FTimerHandle IdleStateTimerHandle;
	FTimerHandle JogFalseTimerHandle;
	FTimerDelegate TimerDel;

	FTimerHandle JogToIdleimerHandle;

public:
	// Mesh를 적용하기 위한 함수
	void SetContentMesh(const TCHAR* ContentPath);

	// Animation을 적용하기 위한 함수
	void SetContentAnim(const TCHAR* ContentPath, bool IsLoop);

	// TextRenderCompoent에 아이디를 적용하고, 캐릭터마다 아이디를 할당하기 위한 함수
	void SetcharID(FString charID);

	// 캐릭터의 위치를 이동시키기 위한 함수
	void MoveLocation(FVector playervector, FRotator playerrotator);

	// 캐릭터의 상태를 변환하기 위한 함수
	//UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	UFUNCTION()
	void FUpdateState(int32 stateNum);

	UFUNCTION()
	void FJogStateStop();

	void FJogFalse();

	void FJogToIdle();

	/*-----------------------------------------------*/
	
};
