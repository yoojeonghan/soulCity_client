// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "kelsaik_20180623Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// Akelsaik_20180623Character

Akelsaik_20180623Character::Akelsaik_20180623Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	CharIDText = CreateDefaultSubobject<UTextRenderComponent>(FName("CharIDText"));

	// 공격판정을 위한 콜리전박스. 한 콜리전박스를 위치랑 스케일 바꿔서 여러 공격에 재활용되게끔 함.
	AttackBox = CreateDefaultSubobject<UBoxComponent>(FName("AttackBox"));
	AttackBox->IsCollisionEnabled();
	AttackBox->OnComponentBeginOverlap.AddDynamic(this, &Akelsaik_20180623Character::BeginOverlap);
	AttackBox->OnComponentEndOverlap.AddDynamic(this, &Akelsaik_20180623Character::EndOverlap);

	IsIDText = false;
	Isfalling = false;

	IsUpdateState = false;
	IsJog = false;
	IsMoving = false;
	IsAttack = false;
	WasAttackDamage = true;
	IsAttackBound = false;
	IsAttack2 = false;

	CurrentLocation = FVector(0, 0, 0);
	CharacterMeshComponet = this->GetMesh();

	IdleAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Idle.Idle'")));
	JogAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jog_Fwd.Jog_Fwd'")));
	StartAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/LevelStart.LevelStart'")));
	ChangeToIdleFromJogAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jog_Fwd_Stop.Jog_Fwd_Stop'")));
	JumpAnimAsset_JumpStart = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Start.Jump_Start'")));
	JumpAnimAsset_Jumping = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Apex.Jump_Apex'")));
	JumpAnimAsset_JumpLand = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Land.Jump_Land'")));
	JumpAnimAsset_JumpEnd = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Jump_Recovery.Jump_Recovery'")));
	JumpMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, TEXT("AnimMontage'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/JumpMontage.JumpMontage'")));
	Attack1Montage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, TEXT("AnimMontage'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/PrimaryMelee_D_Slow_Montage.PrimaryMelee_D_Slow_Montage'")));
	Attack2Montage = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Ability_AttackWolves_Cast_R.Ability_AttackWolves_Cast_R'")));
	
	this->Tags.Add(FName("UserCharacter"));
}

//////////////////////////////////////////////////////////////////////////
// Input

void Akelsaik_20180623Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &Akelsaik_clientCharacter::ToServerJumpState);

	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &Akelsaik_clientCharacter::ToServerJumpStopState);

	PlayerInputComponent->BindAxis("MoveForward", this, &Akelsaik_20180623Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &Akelsaik_20180623Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &Akelsaik_20180623Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &Akelsaik_20180623Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &Akelsaik_20180623Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &Akelsaik_20180623Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &Akelsaik_20180623Character::OnResetVR);

	RootComponent = this->GetRootComponent();

	//InitialStamina = 100.0f;
	//CurrentStamina = InitialStamina;
}

void Akelsaik_20180623Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void Akelsaik_20180623Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
	//ToServerState(4);
}

void Akelsaik_20180623Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void Akelsaik_20180623Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void Akelsaik_20180623Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void Akelsaik_20180623Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && CharStateNum != 6 && !IsAttack)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		//CharacterMeshComponet->PlayAnimation(JogAnimAsset, true);
		//ToServerState(1);
	}
	if (IsAttack)
	{
		//FUpdateState(6);
	}
}

void Akelsaik_20180623Character::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && CharStateNum != 6 && !IsAttack)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
		//CharacterMeshComponet->PlayAnimation(JogAnimAsset, true);
		//ToServerState(1);
	}
	if (IsAttack)
	{
		//FUpdateState(6);
	}
}

void Akelsaik_20180623Character::ToServerState(int32 stateNum)
{
	if (UDPActor->IsSocket)
	{
		UDPActor->ToServerState(stateNum);
	}
}

// 최초 액터 시작 시 UDPActor를 정의합니다.
void Akelsaik_20180623Character::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AUDPActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		UDPActor = *ActorItr;
	}

	for (TActorIterator<AMatchingActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		MatchingActor = *ActorItr;
	}

	TArray<UParticleSystemComponent*> comps;

	GetComponents(comps);

	for (int i = 0; i < comps.Num(); i++)
	{
		AttackBox2 = Cast<UParticleSystemComponent>(comps[i]);
	}

	if (GetWorld())
	{
		if (CharIDText)
		{
			CharIDText->SetRelativeLocation(FVector(0, 0, 85));
			CharIDText->SetWorldRotation(FRotator(0, 180, 0));
			CharIDText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
			CharIDText->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

			FString RandCharNum;
			bool IsSession = true;
			FFileHelper::LoadFileToString(RandCharNum, *(FPaths::ProjectDir() + FString("GameSession")));

			CharIDText->SetText(FText::FromString(RandCharNum));
			// 80, 30, 0
			// 스케일 3
			AttackBox->SetRelativeTransform(FTransform(FQuat(0, 0, 0, 0), FVector(80, 30, 0), FVector(3, 3, 3)));
			AttackBox->SetHiddenInGame(false);
			AttackBox->SetVisibility(false);
		}

		CharacterMeshComponet->PlayAnimation(IdleAnimAsset, true);

		TimerDel.BindUFunction(this, FName("FJogStateStop"), 0);
		GetWorldTimerManager().SetTimer(JogFalseTimerHandle, this, &Akelsaik_20180623Character::FJogFalse, 0.25f, true);

		this->PawnClientRestart();

		//FUpdateState(2);
		//ToServerState(4);
		//FUpdateState(4);

		//this->SetupPlayerInputComponent(this->CreatePlayerInputComponent());
	}

}

void Akelsaik_20180623Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool WasFalling = Isfalling;
	Isfalling = this->GetCharacterMovement()->IsFalling();

	if (WasFalling != Isfalling)
	{
		if (Isfalling)
		{
			ToServerState(4);
			FUpdateState(4);
		}
		else
		{
			ToServerState(5);
			FUpdateState(5);
		}
	}

	FVector TempLocation = CurrentLocation;
	CurrentLocation = this->GetActorLocation();

	if (!TempLocation.Equals(CurrentLocation))
	{
		if (CharStateNum != 2)
		{
			ToServerState(1);
			FUpdateState(1);
		}
	}

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

	//AttackBox->SetRelativeTransform(FTransform(FQuat(0, 0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0)));

	if (IsUpdateState)
	{
		switch (CharStateNum)
		{
		case 0: // IDLE
			IsAttack = false;
			CharacterMeshComponet->PlayAnimation(IdleAnimAsset, true);
			break;

		case 1: // JOG
			IsAttack = false;
			CharacterMeshComponet->PlayAnimation(JogAnimAsset, true);
			GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 0.5f, true);
			break;

		case 2: // START
			CharacterMeshComponet->PlayAnimation(StartAnimAsset, false);
			GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 5.0f, false);
			break;

		case 3: // JOG->IDLE
			CharacterMeshComponet->PlayAnimation(ChangeToIdleFromJogAsset, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &Akelsaik_20180623Character::FJogToIdle, 2.2f, false);
			break;

		case 4: // JUMP
			Jump();
			CharacterMeshComponet->PlayAnimation(JumpMontage, false);
			break;

		case 5: // JUMP->IDLE
			CharacterMeshComponet->PlayAnimation(JumpAnimAsset_JumpEnd, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &Akelsaik_20180623Character::FJogToIdle, 1.1f, false);
			break;

		case 6: //ATTACK1
			ToServerState(6);
			AttackBox->SetRelativeTransform(FTransform(FQuat(0, 0, 0, 0), FVector(80, 30, 0), FVector(3, 3, 3)));
			CharacterMeshComponet->PlayAnimation(Attack1Montage, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &Akelsaik_20180623Character::FJogToIdle, 1.0f, false);
			break;

		case 7: //ATTACK2
			ToServerState(7);
			CharacterMeshComponet->PlayAnimation(Attack2Montage, false);
			GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &Akelsaik_20180623Character::FJogToIdle, 1.833f, false);
			break;
		}
		IsUpdateState = false;
	}
}

void Akelsaik_20180623Character::ToServerJumpState()
{
	ToServerState(4);
	FUpdateState(4);
}

void Akelsaik_20180623Character::ToServerJumpStopState()
{
	ToServerState(5);
	FUpdateState(5);
}

void Akelsaik_20180623Character::Attack1()
{
	if (!IsAttack)
	{
		FUpdateState(6);
		//AttackBox->SetRelativeTransform(FTransform(FQuat(0,0,0,0), FVector(80, 30, 0), FVector(3, 3, 3)));
		IsAttack = true;
		//CharacterMeshComponet->PlayAnimation(Attack1Montage, false);
	}

}

void Akelsaik_20180623Character::Attack2()
{
	if (!IsAttack)
	{
		FUpdateState(7);
		//AttackBox->SetRelativeTransform(FTransform(FQuat(0,0,0,0), FVector(80, 30, 0), FVector(3, 3, 3)));
		IsAttack = true;
		AttackBox2->SetRelativeLocation(FVector(210, 0, 120));
		AttackBox2->SetWorldScale3D(FVector(30, 30, 30));
		AttackBox2->SetVisibility(true);
		AttackBox2->bGenerateOverlapEvents = true;
		//CharacterMeshComponet->PlayAnimation(Attack1Montage, false);
	}
}

void Akelsaik_20180623Character::FUpdateState(int32 stateNum)
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
			
			if (CharStateNum == 6)
			{
				WasAttackDamage = false;
				AttackBound();
			}
		}
	}
}

void Akelsaik_20180623Character::FJogStateStop()
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

void Akelsaik_20180623Character::FJogFalse()
{
	IsJog = false;
}

void Akelsaik_20180623Character::FJogToIdle()
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
			AttackBox2->SetWorldScale3D(FVector(0,0,0));
			AttackBox2->bGenerateOverlapEvents = false;
		}
		FUpdateState(0);
		GetWorldTimerManager().ClearTimer(JogToIdleimerHandle);
	}
}

void Akelsaik_20180623Character::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->ActorHasTag("Monster"))
	{
		IsAttackBound = true;
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MyCharacter] Attack Bound"));
		}
	}
}

void Akelsaik_20180623Character::EndOverlap(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Monster"))
	{
		IsAttackBound = false;
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MyCharacter] NOT Attack Bound"));
		}
	}
}

void Akelsaik_20180623Character::BeginOverlap2(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor->ActorHasTag("Monster"))
	{
		IsAttackBound = true;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MyCharacter] Attack Bound"));
		}
	}
}

void Akelsaik_20180623Character::EndOverlap2(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Monster"))
	{
		IsAttackBound = false;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MyCharacter] NOT Attack Bound"));
		}
	}
}

void Akelsaik_20180623Character::AttackBound()
{
	Akelsaik_20180623GameMode* MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());

	if (!WasAttackDamage && IsAttackBound && CharStateNum == 6)
	{
		switch (CharStateNum)
		{

		case 6:
			if (GEngine)
			{
				float Damage = 0.05;

				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[MyCharacter] Attack Monster"));
				WasAttackDamage = true;
				UDPActor->RequestMonsterAttack(Damage);
				//MyGameMode->UpdateMonsterHPBar(Damage);
			}
			break;
		}
	}
}