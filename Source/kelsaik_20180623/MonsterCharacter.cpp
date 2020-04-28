// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterCharacter.h"
#include "UDPActor.h"
#include "kelsaik_20180623Character.h"

// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	IsJog = false;
	IsActive = false;
	StateNum = 0;
	WasAttackDamage = true;
	IsAttackBound = false;
	IsActiveUI = false;

	IdleAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Idle.Idle'")));
	ActiveIdleAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Idle_Biped.Idle_Biped'")));
	StartAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_Enrage_Start.Ability_Enrage_Start'")));
	JogAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Jog_Quad_Fwd.Jog_Quad_Fwd'")));
	Attack1Montage = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Biped_Melee_A.Attack_Biped_Melee_A'")));
	//AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_Enrage_End.Ability_Enrage_End'
	DieAnimAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_Enrage_End.Ability_Enrage_End'")));
	//AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Stun_Idle.Stun_Idle'
	NuckbackAsset = Cast<UAnimationAsset>(StaticLoadObject(UAnimationAsset::StaticClass(), NULL, TEXT("AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Stun_Idle.Stun_Idle'")));
	// 공격판정을 위한 콜리전박스. 한 콜리전박스를 위치랑 스케일 바꿔서 여러 공격에 재활용되게끔 함.
	AttackBox = CreateDefaultSubobject<UBoxComponent>(FName("AttackBox"));
	AttackBox->IsCollisionEnabled();
	AttackBox->SetVisibility(false);
	AttackBox->SetActive(true);
	AttackBox->OnComponentBeginOverlap.AddDynamic(this, &AMonsterCharacter::BeginOverlap);
	AttackBox->OnComponentEndOverlap.AddDynamic(this, &AMonsterCharacter::EndOverlap);
	AttackBox->SetRelativeTransform(FTransform(FQuat(0, 0, 0, 0), FVector(116.914551, 33.712662, 0), FVector(2, 2, 2)));

	this->Tags.Add(FName("Monster"));
	Http = &FHttpModule::Get();
}

void AMonsterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		TimerDel.BindUFunction(this, FName("FJogStateStop"), 0);
		CharacterMeshComponet = this->GetMesh();
		RootComponent = this->GetRootComponent();
		MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());
		CurrentLocation = this->GetActorLocation();
		CurrentRotator = this->GetActorRotation();

		FString sessiontemp;
		FFileHelper::LoadFileToString(sessiontemp, *(FPaths::ProjectDir() + FString("GameSession")));
		PlayerID = sessiontemp;
	}

	for (TActorIterator<AUDPActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		UDPActor = *ActorItr;
	}

	for (TActorIterator<Akelsaik_20180623Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		MyCharacter = *ActorItr;
	}

	for (TActorIterator<AItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ItemActor = *ActorItr;
	}

	//ItemActor
}

// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsActive && !IsActiveUI)
	{
		MyGameMode->VisibleMonsterUI(true);
		IsActiveUI = true;
	}

	destinationLocation = FMath::VInterpTo(this->GetActorLocation(), CurrentLocation, DeltaTime, 10.0f);
	this->SetActorLocation(destinationLocation);
	this->SetActorRotation(FMath::Lerp(this->GetActorRotation(), CurrentRotator, 0.2f));

	if (IsUpdateState)
	{
		switch (StateNum)
		{
			//AudioComponent->Play();
			case 0: // IDLE
				CharacterMeshComponet->PlayAnimation(IdleAnimAsset, true);
				break;

			case 1: // JOG
				CharacterMeshComponet->PlayAnimation(JogAnimAsset, true);
				GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 0.5f, true);
				break;

			case 2: // START
				CharacterMeshComponet->PlayAnimation(StartAnimAsset, false);
				GetWorldTimerManager().SetTimer(IdleStateTimerHandle, TimerDel, 1.3f, false);
				break;

			case 3: // JOG->IDLE
				//CharacterMeshComponet->PlayAnimation(ChangeToIdleFromJogAsset, false);
				//GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AWorldCharacter::FJogToIdle, 2.2f, false);
				break;

			case 4: // ActiveIDLE
				CharacterMeshComponet->PlayAnimation(ActiveIdleAnimAsset, true);
				break;
		
			case 5: // 넉백
				CharacterMeshComponet->PlayAnimation(NuckbackAsset, false);
				GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AMonsterCharacter::FJogToIdle, 3.667f, false);
				break;
				
			case 6: //ATTACK1
				//AttackBox->SetRelativeTransform(FTransform(FQuat(0,0,0,0), FVector(151.06485, 6.834819, -40), FVector(2, 3, 3)));
				CharacterMeshComponet->PlayAnimation(Attack1Montage, false);
				GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AMonsterCharacter::FJogToIdle, 1.0f, false);
				break;

			case 7: // DIEING
				CharacterMeshComponet->PlayAnimation(DieAnimAsset, false);
				GetWorldTimerManager().SetTimer(JogToIdleimerHandle, this, &AMonsterCharacter::FJogToIdle, 7.0f, false);
				break;

			case 8: // DIE
				this->SetActorHiddenInGame(true);
				MyGameMode->VisibleMonsterUI(false);
				ItemActor->VisibleItem();
				MyHttpCall();
				break;
		}

		IsUpdateState = false;
	}
}

// Called to bind functionality to input
void AMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


}

// 캐릭터의 상태를 변환하기 위한 함수
void AMonsterCharacter::FUpdateState(int32 stateNum)
{
	if (StateNum != stateNum)
	{
		StateNum = stateNum;
		IsUpdateState = true;
		
		if (stateNum == 2)
		{
			IsActive = true;
		}

		if (stateNum == 6)
		{
			WasAttackDamage = false;
			AttackBound();
		}
	}
}

void AMonsterCharacter::FJogFalse()
{
	IsJog = false;
}

void AMonsterCharacter::FJogStateStop()
{
	if (StateNum == 1)
	{
		if (!IsJog)
		{
			FUpdateState(3);
		}
	}
	else if (StateNum == 2)
	{
		FUpdateState(4);
	}
}

void AMonsterCharacter::FJogToIdle()
{
	if (StateNum == 7)
	{
		FUpdateState(8);
	}
	else
	{
		if (StateNum == 6)
		{
			IsAttack = false;
		}
		FUpdateState(4);
		GetWorldTimerManager().ClearTimer(JogToIdleimerHandle);
	}

}

void AMonsterCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (GEngine)
	{
		if (OtherActor->ActorHasTag(FName("UserCharacter")))
		{
			IsAttackBound = true;
			if (GEngine)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("[Monster] Attack Bound"));
			}
		}

		if (OtherComp->ComponentHasTag(FName("attack2")))
		{
			if(MyCharacter->IsAttack2)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("[Monster] Attack Bound"));
				UDPActor->RequestMonsterAttack2(0.05f);
			}

		}
	}
}

void AMonsterCharacter::EndOverlap(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GEngine)
	{
		if (OtherActor->ActorHasTag(FName("UserCharacter")))
		{
			IsAttackBound = false;
			if (GEngine)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("[Monster] NOT Attack Bound"));
			}
		}

		if (OtherComp->ComponentHasTag(FName("attack2")))
		{
			if (MyCharacter->IsAttack2)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("[Monster] Not Attack Bound"));
				//UDPActor->RequestMonsterAttack2(0.05f);
			}
		}
	}
}

void AMonsterCharacter::AttackBound()
{
	Akelsaik_20180623GameMode* MyGameMode = Cast<Akelsaik_20180623GameMode>(GetWorld()->GetAuthGameMode());

	if (!WasAttackDamage && IsAttackBound && StateNum == 6)
	{
		switch (StateNum)
		{
			case 6:
				if (GEngine)
				{
					float Damage = 0.05;
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("[Monster] Attack UserCharacter"));
					WasAttackDamage = true;
					UDPActor->RequestUserAttack(Damage);
				}
				break;
		}
	}
}

void AMonsterCharacter::MonsterDie()
{

}

void AMonsterCharacter::MonsterMove(FVector monsterVector, FRotator monsterRotator)
{
	CurrentLocation = monsterVector;
	CurrentRotator = monsterRotator;
}

void AMonsterCharacter::MonsterMove(FRotator monsterRotator)
{
	CurrentRotator = monsterRotator;
}

void AMonsterCharacter::MyHttpCall()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, "Request");
	}
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMonsterCharacter::OnResponseReceived);
	//This is the url on which to process the request
	FString RequestURL = "http://52.78.90.90:80/SoulCity_Ranking.php?";
	RequestURL.Append("PlayerID=");
	RequestURL.Append(PlayerID);
	Request->SetURL(RequestURL);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	//Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("Content-Type", TEXT("text/plain"));
	//Request->Set
	//SetContentAsString
	//Request->SetContentAsString(ContentJsonString);
	Request->ProcessRequest();
}

/*Assigned function on successfull http call*/
void AMonsterCharacter::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ResponseString = Response->GetContentAsString();

	if (GEngine)
	{
		if (bWasSuccessful)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ResponseString);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FALSE"));
		}
	}
}