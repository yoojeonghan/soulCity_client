// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "kelsaik_20180623GameMode.h"
#include "kelsaik_20180623Character.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "TCPActor.h"
#include "UDPActor.h"
#include "Json.h"
#include "JSONObjectConverter.h"
#include "MatchingActor.h"
#include "MonsterCharacter.h"

Akelsaik_20180623GameMode::Akelsaik_20180623GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerHUDClass = AHUD::StaticClass();
	Http = &FHttpModule::Get();
}

void Akelsaik_20180623GameMode::BeginPlay()
{
	Super::BeginPlay();

	// CharID;
	bool IsSession = true;
	FFileHelper::LoadFileToString(CharID, *(FPaths::ProjectDir() + FString("GameSession")));

	if (PlayerHUDClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), PlayerHUDClass);

		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();

			UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RecvChatBox"));
		}
	}
	
	for (TActorIterator<ATCPActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		TCPActor = *ActorItr;
	}
	

	for (TActorIterator<Akelsaik_20180623Character> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		MyCharacter = *ActorItr;

		UTextBlock *IDText = Cast<UTextBlock>(CurrentWidget->GetWidgetFromName("myID"));
		FString ID = CharID;
		IDText->SetText(FText::FromString(ID));
	}

	
	for (TActorIterator<AMatchingActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		MatchingActor = *ActorItr;
	}
	
	for (TActorIterator<AUDPActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		UDPActor = *ActorItr;
	}

	for (TActorIterator<AMonsterCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (CurrentWidget)
		{
			UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RecvChatBox"));
			FString OutputString = TEXT("[알림] 램페이지의 성소에 입장하였습니다.");
			FText OutputText = FText::FromString(OutputString);

			if (RecvChatBox)
			{
				UTextBlock *RecvChatTextBlock = NewObject<UTextBlock>();

				if (RecvChatTextBlock)
				{
					RecvChatTextBlock->SetText(OutputText);
					RecvChatBox->AddChildToVerticalBox(RecvChatTextBlock);
				}
			}
		}
	}

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	MyGameSettings->SetScreenResolution(FIntPoint(960, 540));
	MyGameSettings->SetFullscreenMode(EWindowMode::Windowed);
	MyGameSettings->SetVSyncEnabled(true);
	MyGameSettings->ApplySettings(true);

	//GetGameUserSettings->SetFullscreenMode(Windowed);
}

void Akelsaik_20180623GameMode::FSendChatting()
{
	UEditableTextBox *ChatTextBox = Cast<UEditableTextBox>(CurrentWidget->GetWidgetFromName("SendChatBox"));

	if (ChatTextBox)
	{
		FText Sendtext = ChatTextBox->GetText();
		if (!Sendtext.IsEmpty())
		{
			TCPActor->SendingChat(Sendtext);
			FText Cleartext = FText();
			ChatTextBox->SetText(Cleartext);
		}
	}
}

void Akelsaik_20180623GameMode::FReceivedChatting(FString Recvingtext, FString Sender)
{
	if (CurrentWidget)
	{
		UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RecvChatBox"));

		FString OutputString = TEXT("[전체] [") + Sender + TEXT("] : ") + Recvingtext;
		FText OutputText = FText::FromString(OutputString);

		if (RecvChatBox)
		{
			UTextBlock *RecvChatTextBlock = NewObject<UTextBlock>();

			if (RecvChatTextBlock)
			{
				RecvChatTextBlock->SetText(OutputText);
				RecvChatBox->AddChildToVerticalBox(RecvChatTextBlock);
			}
		}
	}
}

void Akelsaik_20180623GameMode::FLoginUser(FString LoginUser)
{
	if (CurrentWidget)
	{
		UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RecvChatBox"));
		FString OutputString = TEXT("[알림] ") + LoginUser + TEXT("님이 접속하였습니다.");
		FText OutputText = FText::FromString(OutputString);

		if (RecvChatBox)
		{
			UTextBlock *RecvChatTextBlock = NewObject<UTextBlock>();

			if (RecvChatTextBlock)
			{
				RecvChatTextBlock->SetText(OutputText);
				RecvChatBox->AddChildToVerticalBox(RecvChatTextBlock);
			}
		}
	}
}

void Akelsaik_20180623GameMode::FLogoutUser(FString LoginUser)
{
	if (CurrentWidget)
	{
		UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RecvChatBox"));
		FString OutputString = TEXT("[알림] ") + LoginUser + TEXT("님이 접속을 종료하였습니다.");
		FText OutputText = FText::FromString(OutputString);

		if (RecvChatBox)
		{
			UTextBlock *RecvChatTextBlock = NewObject<UTextBlock>();

			if (RecvChatTextBlock)
			{
				RecvChatTextBlock->SetText(OutputText);
				RecvChatBox->AddChildToVerticalBox(RecvChatTextBlock);
			}
		}
	}
}

void Akelsaik_20180623GameMode::FGameExit()
{
	UDPActor->SessionLogout();
	TCPActor->SessionLogout();
	GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
}

void Akelsaik_20180623GameMode::FAttack1()
{
	if (MyCharacter)
	{
		MyCharacter->Attack1();
	}
}

void Akelsaik_20180623GameMode::FAttack2()
{
	if (MyCharacter)
	{
		MyCharacter->Attack2();
	}
}

void Akelsaik_20180623GameMode::FRequestMatching()
{
	if (MatchingActor)
	{
		MatchingActor->RequestMatching();
	}
}

void Akelsaik_20180623GameMode::FRequestMatchingCansel()
{
	if (MatchingActor)
	{
		MatchingActor->CanselMatching();
	}
}

void Akelsaik_20180623GameMode::RequestMonsterBegin()
{
	if (UDPActor)
	{
		UDPActor->RequestMonsterBegin();
	}
}

void Akelsaik_20180623GameMode::RecevingMatch()
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
	}

	if (GetWorld())
	{
		if (GetWorld()->GetFirstPlayerController())
		{
			GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeUIOnly());
			GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());
		}

		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/SoulCave/Maps/LV_Soul_Cave_Mobile"), TRAVEL_Absolute);
	}
}

void Akelsaik_20180623GameMode::UpdateMonsterHPBar(float hP)
{
	if (CurrentWidget)
	{
		UProgressBar* MonsterHPBar = Cast<UProgressBar>(CurrentWidget->GetWidgetFromName("MonsterHPBar"));

		if (hP > 0)
		{
			MonsterHPBar->SetPercent(hP);
		}
		else
		{
			MonsterHPBar->SetPercent(0.0f);
		}
	}
}

void Akelsaik_20180623GameMode::UpdateMyHPBar(float hP)
{
	if (CurrentWidget)
	{
		UProgressBar* MyHPBar = Cast<UProgressBar>(CurrentWidget->GetWidgetFromName("MyHPBar"));
		MyHPBar->SetPercent(hP);
	}
}

void Akelsaik_20180623GameMode::VisibleMonsterUI(bool isVisible)
{
	UVerticalBox *MonsterBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("MonsterBox"));

	if (isVisible)
	{
		MonsterBox->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MonsterBox->SetVisibility(ESlateVisibility::Hidden);
	}
}

void Akelsaik_20180623GameMode::CreateTeamUI(TArray<FString> playerNameArray)
{
	for (int i = 0; i < playerNameArray.Num(); i++)
	{
		PlayerNameArray.Add(playerNameArray[i]);
	}

	if (CurrentWidget)
	{
		UVerticalBox *TeamHPList = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("TeamHPList"));

		for (int j = 0; j < PlayerNameArray.Num(); j++)
		{
			FText UserName = FText::FromString(PlayerNameArray[j]);

			if (TeamHPList)
			{
				UTextBlock *UserNameBlock = NewObject<UTextBlock>();

				if (UserNameBlock)
				{
					UserNameBlock->SetText(UserName);
					TeamHPList->AddChildToVerticalBox(UserNameBlock);

					UProgressBar *UserHPBar = NewObject<UProgressBar>();
					ProgressArray.Add(UserHPBar);
					
					if (UserHPBar)
					{
						UserHPBar->FillColorAndOpacity = FLinearColor(0.619792f, 0.006422f, 0.012587f, 1.0f);
						UserHPBar->SetPercent(1.0f);

						TeamHPList->AddChildToVerticalBox(UserHPBar);
					}
				}
			}
		}
	}
}

void Akelsaik_20180623GameMode::UpdateUserHPBar(FString userID, float hP)
{
	if (CurrentWidget)
	{
		UVerticalBox *TeamHPList = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("TeamHPList"));
		
		for (int z = 0; z < PlayerNameArray.Num(); z++)
		{
			if (PlayerNameArray[z].Equals(userID))
			{
				ProgressArray[z]->SetPercent(hP);
			}
		}
	}
}

void Akelsaik_20180623GameMode::UserLogin()
{
	if (CurrentWidget)
	{
		UEditableText *IDTextBox = Cast<UEditableText>(CurrentWidget->GetWidgetFromName("IDTextBox"));

		if (IDTextBox)
		{
			FText IDtext = IDTextBox->GetText();
			if (!IDtext.IsEmpty())
			{
				FText Cleartext = FText();
				FFileHelper::SaveStringToFile(IDtext.ToString(), *(FPaths::ProjectDir() + FString("GameSession")));
			}
		}
	}
}

void Akelsaik_20180623GameMode::ShowInventory()
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &Akelsaik_20180623GameMode::OnResponseReceived);
	//This is the url on which to process the request
	FString RequestURL = "http://52.78.90.90:80/SoulCity_ShowItem.php?";
	RequestURL.Append("PlayerID=");
	RequestURL.Append(CharID);
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
void Akelsaik_20180623GameMode::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ResponseString = Response->GetContentAsString();
	ItemNum = FCString::Atoi(*ResponseString);
}

int32 Akelsaik_20180623GameMode::GetItemNum()
{
	return ItemNum;
}

void Akelsaik_20180623GameMode::ShowRanking()
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &Akelsaik_20180623GameMode::OnResponseReceived2);
	//This is the url on which to process the request
	FString RequestURL = "http://52.78.90.90:80/SoulCity_ShowRanking.php";
	//RequestURL.Append("PlayerID=");
	//RequestURL.Append(CharID);
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
void Akelsaik_20180623GameMode::OnResponseReceived2(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FString ResponseString = Response->GetContentAsString();
	
	TSharedPtr<FJsonObject> RecvJsonObject = MakeShareable(new FJsonObject);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	FJsonSerializer::Deserialize(Reader, RecvJsonObject);

	TArray<TSharedPtr<FJsonValue>> objArray = RecvJsonObject->GetArrayField(TEXT("Ranking"));

	RankingArray.Empty();

	for (int32 i = 0; i < objArray.Num(); i++)
	{
		TSharedPtr<FJsonObject> Player = objArray[i]->AsObject();
		
		FString PlayerID = Player->GetStringField("UserID");
		int32 PlayerScore = Player->GetIntegerField("Score");

		FString RankString = "      " +PlayerID +"             "+FString::FromInt(PlayerScore);

		RankingArray.Add(RankString);

		/*
		//RankingBox

		*/
	}

}

TArray<FString> Akelsaik_20180623GameMode::GetRanking()
{
	return RankingArray;
}

void Akelsaik_20180623GameMode::CreateRankStringObject()
{
	if (CurrentWidget)
	{
		UVerticalBox *RecvChatBox = Cast<UVerticalBox>(CurrentWidget->GetWidgetFromName("RankingBox"));

		for (int i = 0; i < RankingArray.Num(); i++)
		{
			FText OutputText = FText::FromString(RankingArray[i]);

			if (RecvChatBox)
			{
				UTextBlock *RecvChatTextBlock = NewObject<UTextBlock>();

				if (RecvChatTextBlock)
				{
					RecvChatTextBlock->SetText(OutputText);
					RecvChatBox->AddChildToVerticalBox(RecvChatTextBlock);
				}
			}
		}

	}
}