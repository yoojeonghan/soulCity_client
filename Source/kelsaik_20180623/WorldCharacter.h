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
	// WorldCharacter ���Ϳ� ����� SkeletalMeshComponent ����
	USkeletalMeshComponent * CharacterMeshComponet;

	// WorldCharacter ���Ϳ� ����� MovementComponent ����
	UCharacterMovementComponent* CharacterMovementComponet;

	//	APlayerController *CharacterController;

	// WorldCharacter ������ ���̵� ����. ���а� �� �Ӹ����� ǥ�õ� TextRenderComponent�� �����
	FString CharID;

	// �� ĳ���͸��� �Ӹ� ���� ����� TextReanderComponent ����
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CharIDText;

	// ĳ���� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CharStateNum;

	// ĳ������ ��ǥ�� �ޱ� ���� Vector �ڷ���
	FVector PlayerVector;

	// ĳ������ ȸ������ �ޱ� ���� Rotator �ڷ���
	FRotator PlayerRotator;

	// TextRenderCompoent�� �� ���� �����ϱ� ���� ���� bool ����
	bool IsIDText;

	bool IsWorld;

	bool IsBegining;

	bool IsJog;

	// �ִϸ��̼� ����

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
	// Mesh�� �����ϱ� ���� �Լ�
	void SetContentMesh(const TCHAR* ContentPath);

	// Animation�� �����ϱ� ���� �Լ�
	void SetContentAnim(const TCHAR* ContentPath, bool IsLoop);

	// TextRenderCompoent�� ���̵� �����ϰ�, ĳ���͸��� ���̵� �Ҵ��ϱ� ���� �Լ�
	void SetcharID(FString charID);

	// ĳ������ ��ġ�� �̵���Ű�� ���� �Լ�
	void MoveLocation(FVector playervector, FRotator playerrotator);

	// ĳ������ ���¸� ��ȯ�ϱ� ���� �Լ�
	//UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	UFUNCTION()
	void FUpdateState(int32 stateNum);

	UFUNCTION()
	void FJogStateStop();

	void FJogFalse();

	void FJogToIdle();

	/*-----------------------------------------------*/
	
};
