// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
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
#include "UDPActor.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "kelsaik_20180623GameMode.h"
#include "MatchingActor.h"

/*--------------------------------------------------*/

#include "kelsaik_20180623Character.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPchar, Warning, All);

UCLASS(config=Game)
class Akelsaik_20180623Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	Akelsaik_20180623Character();

	virtual void Tick(float DeltaTime) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void BeginPlay();

	//void EndPlay(const EEndPlayReason::Type EndPlayReason);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	UPROPERTY(VisibleDefaultsOnly, Category = Text)
	// 캐릭터 머리 위에 아이디 텍스트를 띄우기 위해 아이디 재정의
	int32 RandCharNum;
	// 캐릭터 머리 위에 아이디 텍스트를 띄우기 위한 컴포넌트
	UTextRenderComponent* CharIDText;
	// 컴포넌트를 호출하기 위해 최상위의 RootComponent를 정의합니다.
	USceneComponent* RootComponent;
	// 아이디 텍스트 컴포넌트를 한 번만 호출하기 위해 사용하는 bool 변수
	bool IsIDText;

	bool Isfalling;

	bool IsAttackBound;

	bool IsAttack2;

	FVector CurrentLocation;

	UAnimMontage* Attack1Montage;
	UAnimationAsset* Attack2Montage;

	USkeletalMeshComponent* CharacterMeshComponet;

	UAnimationAsset* IdleAnimAsset;
	UAnimationAsset* JogAnimAsset;
	UAnimationAsset* StartAnimAsset;
	UAnimationAsset* ChangeToIdleFromJogAsset;
	UAnimationAsset* JumpAnimAsset_JumpStart;
	UAnimationAsset* JumpAnimAsset_Jumping;
	UAnimationAsset* JumpAnimAsset_JumpLand;
	UAnimationAsset* JumpAnimAsset_JumpEnd;
	UAnimMontage* JumpMontage;

	FTimerHandle IdleStateTimerHandle;
	FTimerHandle JogFalseTimerHandle;
	FTimerDelegate TimerDel;

	FTimerHandle JogToIdleimerHandle;

	/*--------------------------------------------------*/

	// UDPActor의 함수를 실행시키기 위해 UDPActor를 재선언합니다.
	AUDPActor *UDPActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AMatchingActor* MatchingActor;

	/*--------------------------------------------------*/

public:
	// 캐릭터 상태변환 시 UDPActor의 ToServerState를 호출하기 위한 함수
	void ToServerState(int32 stateNum);

	void ToServerJumpState();

	void ToServerJumpStopState();

	UFUNCTION()
	void Attack1();

	UFUNCTION()
	void Attack2();

	// 기존 블루프린트 애니메이션에서 에셋 애니메이션으로 바꾸기 위한 변수

	/*--------------------------------------------------*/

	UPROPERTY(EditAnywhere)
	UBoxComponent* AttackBox;

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* AttackBox2;

	/*--------------------------------------------------*/

	// 캐릭터 상태 정의
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CharStateNum;

	bool IsUpdateState;
	bool IsMoving;
	bool IsJump;
	bool IsJog;
	bool IsAttack;
	bool WasAttackDamage;

	// 캐릭터의 상태를 변환하기 위한 함수
	//UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	UFUNCTION()
	void FUpdateState(int32 stateNum);

	UFUNCTION()
	void FJogStateStop();

	void FJogFalse();

	void FJogToIdle();

	/*--------------------------------------------------*/

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void BeginOverlap2(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap2(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void AttackBound();
};