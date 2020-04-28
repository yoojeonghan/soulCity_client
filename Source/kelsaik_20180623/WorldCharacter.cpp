// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldCharacter.h"


// Sets default values
AWorldCharacter::AWorldCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터의 캡슐컴포넌트 사이즈를 정의합니다.
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 머리 위에 위치할 텍스트 렌더링 컴포넌트를 정의합니다.
	CharIDText = CreateDefaultSubobject<UTextRenderComponent>(FName("CharIDText"));

	// 텍스트 렌더링 컴포넌트를 한 번만 호출하기 위해 사용하는 bool 변수
	IsIDText = false;
	CharacterMeshComponet = this->GetMesh();

	if (CharacterMeshComponet)
	{
		SetContentMesh(TEXT("SkeletalMesh'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Meshes/Shinbi.Shinbi'"));
	}

	Attack1Montage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, TEXT("AnimMontage'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/PrimaryMelee_D_Slow_Montage.PrimaryMelee_D_Slow_Montage'")));
	Attack2Montage = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Ability_AttackWolves_Cast_R.Ability_AttackWolves_Cast_R'")));

	IdleAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Idle.Idle'")));
	JogAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jog_Fwd.Jog_Fwd'")));
	StartAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/LevelStart.LevelStart'")));
	ChangeToIdleFromJogAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jog_Fwd_Stop.Jog_Fwd_Stop'")));
	JumpAnimAsset_JumpStart = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Start.Jump_Start'")));
	JumpAnimAsset_Jumping = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Apex.Jump_Apex'")));
	JumpAnimAsset_JumpLand = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Land.Jump_Land'")));
	JumpAnimAsset_JumpEnd = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Recovery.Jump_Recovery'")));
	JumpMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, TEXT("AnimMontage'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/JumpMontage.JumpMontage'")));

	CharacterMovementComponet = this->GetCharacterMovement();
	CharacterMovementComponet->CharacterOwner = this;

	CharacterMovementComponet->JumpZVelocity = 600.0f;
	CharacterMovementComponet->AirControl = 0.2f;
	IsUpdateState = false;
	IsWorld = false;
	IsMoving = false;
	IsAttack2 = false;

	speed = 150.0f;

	PlayerVector = FVector(0, 0, 0);
	CurrentLocation = FVector(0, 0, 0);
}

// TextRenderCompoent에 아이디를 적용하고, 캐릭터마다 아이디를 할당하기 위한 함수
void AWorldCharacter::SetcharID(FString charID)
{
	if (charID.IsEmpty())
	{
		CharID.Empty();
		IsIDText = false;
		PlayerVector = FVector(0, -10000, 0);
	}
	else
	{
		CharID = charID;
	}
}

// Called when the game starts or when spawned
void AWorldCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		if (CharIDText)
		{
			CharIDText->SetRelativeLocation(FVector(0, 0, 85));
			CharIDText->SetWorldRotation(FRotator(0, 180, 0));
			CharIDText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
			CharIDText->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		CharacterMeshComponet->PlayAnimation(IdleAnimAsset, true);

		TimerDel.BindUFunction(this, FName("FJogStateStop"), 0);
		GetWorldTimerManager().SetTimer(JogFalseTimerHandle, this, &AWorldCharacter::FJogFalse, 0.25f, true);
	}

	TArray<UParticleSystemComponent*> comps;

	GetComponents(comps);

	for (int i = 0; i < comps.Num(); i++)
	{
		AttackBox2 = Cast<UParticleSystemComponent>(comps[i]);
	}

}

// Called every frame
void AWorldCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsBegining)
	{
		this->SetActorLocation(PlayerVector);
		this->SetActorRotation(PlayerRotator);

		IsBegining = false;
	}

	if (!IsIDText)
	{
		if (!CharID.IsEmpty())
		{
			if (CharIDText)
			{
				CharIDText->SetText(FText::FromString(CharID));
				IsIDText = true;
			}
		}
	}

	if (AttackBox2)
	{
		if (AttackBox2->IsVisible())
		{
			//AttackBox2->RelativeLocation.X+=10;
			AttackBox2->SetRelativeLocation(FVector(AttackBox2->RelativeLocation.X += 50, AttackBox2->RelativeLocation.Y, AttackBox2->RelativeLocation.Z));
			IsAttack2 = true;
			//AttackBox2->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			IsAttack2 = false;
		}
	}
	
	if (!IsWorld)
	{
		if (CharID.IsEmpty())
		{
			SetActorHiddenInGame(true);
		}
		else
		{
			SetActorHiddenInGame(false);
			IsWorld = true;
		}
	}
	
	destinationLocation = FMath::VInterpTo(this->GetActorLocation(), PlayerVector, DeltaTime, 10.0f);
	this->SetActorLocation(destinationLocation);


	this->SetActorRotation(FMath::Lerp(this->GetActorRotation(), PlayerRotator, 0.2f));

	if (IsUpdateState)
	{
		switch (CharStateNum)
		{
		case 0: // IDLE
			CharacterMeshComponet->PlayAnimation(IdleAnimAsset, true);
			break;

		case 1: // JOG
			CharacterMeshComponet->PlayAnimation(JogAnimAsset, true);
			GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 0.5f, true);
			break;

		case 2: // START
			CharacterMeshComponet->PlayAnimation(StartAnimAsset, false);
			GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 5.0f, false);
			break;

		case 3: // JOG->IDLE
			CharacterMeshComponet->PlayAnimation(ChangeToIdleFromJogAsset, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AWorldCharacter::FJogToIdle, 2.2f, false);
			break;

		case 4: // JUMP
			Jump();
			CharacterMeshComponet->PlayAnimation(JumpMontage, false);
			break;

			// 점프 끝나는 판정을 받는 클라이언트가 아니라 보내는 클라이언트가 하도록 변경할 것
		case 5: // JUMP->IDLE
			CharacterMeshComponet->PlayAnimation(JumpAnimAsset_JumpEnd, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AWorldCharacter::FJogToIdle, 1.1f, false);
			break;

		case 6: //ATTACK1
			CharacterMeshComponet->PlayAnimation(Attack1Montage, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AWorldCharacter::FJogToIdle, 1.0f, false);
			break;

		case 7: //ATTACK2
			//ToServerState(7);
			CharacterMeshComponet->PlayAnimation(Attack2Montage, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AWorldCharacter::FJogToIdle, 1.833f, false);
			break;
		}
		IsUpdateState = false;
	}
}

// Mesh를 적용하기 위한 함수
void AWorldCharacter::SetContentMesh(const TCHAR* meshContentPath)
{
	USkeletalMesh* NewMesh = Cast< USkeletalMesh >(StaticLoadObject(USkeletalMesh::StaticClass(), NULL, meshContentPath));

	const FVector MashLoc(0.0f, 0.0f, -97.0f);
	const FRotator MashRot(0.0f, -90.0f, 0.0f);
	const FVector MashSca(1.0f, 1.0f, 1.0f);

	const FTransform MashTransform(MashRot, MashLoc, MashSca);

	if (NewMesh)
	{
		CharacterMeshComponet->SetSkeletalMesh(NewMesh);
		CharacterMeshComponet->SetRelativeTransform(MashTransform);
	}
}

// 애니메이션 블루프린트를 적용하기 위한 함수
void AWorldCharacter::SetContentAnim(const TCHAR* ContentPath, bool IsLoop)
{
	//UAnimationAsset* armAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, ContentPath));

	/*
	if (armAnimAsset)
	{
	CharacterMeshComponet->PlayAnimation(armAnimAsset, IsLoop);
	}
	*/
}

// 캐릭터의 위치를 이동시키기 위한 함수
void AWorldCharacter::MoveLocation(FVector playervector, FRotator playerrotator)
{
	PlayerVector = playervector;
	PlayerRotator = playerrotator;

	IsMoving = true;
}

// 캐릭터의 상태를 변환하기 위한 함수
void AWorldCharacter::FUpdateState(int32 stateNum)
{
	if (CharStateNum != stateNum)
	{
		if (CharStateNum == 4 && stateNum == 1)
		{

		}
		else if (CharStateNum == 5 && stateNum == 1)
		{

		}
		else
		{
			CharStateNum = stateNum;
			IsUpdateState = true;
		}

	}
}

void AWorldCharacter::FJogStateStop()
{
	if (CharStateNum == 1)
	{
		if (!IsJog)
		{
			FUpdateState(3);
		}
	}
	else if (CharStateNum == 2)
	{
		FUpdateState(0);
	}

}

void AWorldCharacter::FJogFalse()
{
	IsJog = false;
}

void AWorldCharacter::FJogToIdle()
{
	if (CharStateNum != 4)
	{
		if (CharStateNum == 6)
		{
			IsAttack = false;
		}
		if (CharStateNum == 7)
		{
			IsAttack = false;
			AttackBox2->SetVisibility(false);
			AttackBox2->SetRelativeLocation(FVector(0, 0, 0));
			AttackBox2->SetWorldScale3D(FVector(0, 0, 0));
			AttackBox2->bGenerateOverlapEvents = false;
		}
		FUpdateState(0);
		GetWorldTimerManager().ClearTimer(JogToIdleimerHandle);
	}

}

// Called to bind functionality to input
void AWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
