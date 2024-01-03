// Done


#include "StarRailPlayerController.h"
#include "UEStarRail/HUD/StarRailHUD.h"
#include "UEStarRail/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "Net/UnrealNetwork.h"
#include "UEStarRail/GameMode/StarRailGameMode.h"
#include "UEStarRail/PlayerState/StarRailPlayerState.h"
#include "UEStarRail/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "UEStarRail/StarRailComponents/CombatComponent.h"
#include "UEStarRail/GameState/StarRailGameState.h"
#include "Components/Image.h"
#include "UEStarRail/HUD/ReturnToMainMenu.h"
#include "UEStarRail/StarRailTypes/Announcement.h"

void AStarRailPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AStarRailPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
		if (StarRailHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				StarRailHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				StarRailHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				StarRailHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				StarRailHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			StarRailHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AStarRailPlayerController::BeginPlay()
{
	Super::BeginPlay();

	StarRailHUD = Cast<AStarRailHUD>(GetHUD());
	ServerCheckMatchState();
}

void AStarRailPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStarRailPlayerController, MatchState);
	DOREPLIFETIME(AStarRailPlayerController, bShowTeamScores);
}

void AStarRailPlayerController::HideTeamScores()
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->RedTeamScore &&
		StarRailHUD->CharacterOverlay->BlueTeamScore &&
		StarRailHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		StarRailHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		StarRailHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		StarRailHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AStarRailPlayerController::InitTeamScores()
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->RedTeamScore &&
		StarRailHUD->CharacterOverlay->BlueTeamScore &&
		StarRailHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		StarRailHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		StarRailHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		StarRailHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void AStarRailPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		StarRailHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AStarRailPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		StarRailHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AStarRailPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void AStarRailPlayerController::CheckPing(float DeltaTime)
{
	if (HasAuthority()) return;
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > HighPingThreshold) // ping is compressed; it's actually ping / 4
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		StarRailHUD && StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->HighPingAnimation &&
		StarRailHUD->CharacterOverlay->IsAnimationPlaying(StarRailHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AStarRailPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AStarRailPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

// Is the ping too high?
void AStarRailPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AStarRailPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AStarRailPlayerController::HighPingWarning()
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->HighPingImage &&
		StarRailHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		StarRailHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		StarRailHUD->CharacterOverlay->PlayAnimation(
			StarRailHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void AStarRailPlayerController::StopHighPingWarning()
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->HighPingImage &&
		StarRailHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		StarRailHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (StarRailHUD->CharacterOverlay->IsAnimationPlaying(StarRailHUD->CharacterOverlay->HighPingAnimation))
		{
			StarRailHUD->CharacterOverlay->StopAnimation(StarRailHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AStarRailPlayerController::ServerCheckMatchState_Implementation()
{
	AStarRailGameMode* GameMode = Cast<AStarRailGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AStarRailPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (StarRailHUD && MatchState == MatchState::WaitingToStart)
	{
		StarRailHUD->AddAnnouncement();
	}
}

void AStarRailPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(InPawn);
	if (StarRailCharacter)
	{
		SetHUDHealth(StarRailCharacter->GetHealth(), StarRailCharacter->GetMaxHealth());
	}
}

void AStarRailPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->HealthBar &&
		StarRailHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		StarRailHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		StarRailHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AStarRailPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->ShieldBar &&
		StarRailHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		StarRailHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		StarRailHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AStarRailPlayerController::SetHUDScore(float Score)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		StarRailHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AStarRailPlayerController::SetHUDDefeats(int32 Defeats)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		StarRailHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AStarRailPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		StarRailHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AStarRailPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		StarRailHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AStarRailPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			StarRailHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		StarRailHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AStarRailPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->Announcement &&
		StarRailHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			StarRailHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		StarRailHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AStarRailPlayerController::SetHUDGrenades(int32 Grenades)
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	bool bHUDValid = StarRailHUD &&
		StarRailHUD->CharacterOverlay &&
		StarRailHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		StarRailHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void AStarRailPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		if (StarRailGameMode == nullptr)
		{
			StarRailGameMode = Cast<AStarRailGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = StarRailGameMode->LevelStartingTime;
		}
		StarRailGameMode = StarRailGameMode == nullptr ? Cast<AStarRailGameMode>(UGameplayStatics::GetGameMode(this)) : StarRailGameMode;
		if (StarRailGameMode)
		{
			SecondsLeft = FMath::CeilToInt(StarRailGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void AStarRailPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (StarRailHUD && StarRailHUD->CharacterOverlay)
		{
			CharacterOverlay = StarRailHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(GetPawn());
				if (StarRailCharacter && StarRailCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(StarRailCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AStarRailPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &AStarRailPlayerController::ShowReturnToMainMenu);

}

void AStarRailPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AStarRailPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AStarRailPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AStarRailPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AStarRailPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AStarRailPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AStarRailPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	if (StarRailHUD)
	{
		if (StarRailHUD->CharacterOverlay == nullptr) StarRailHUD->AddCharacterOverlay();
		if (StarRailHUD->Announcement)
		{
			StarRailHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AStarRailPlayerController::HandleCooldown()
{
	StarRailHUD = StarRailHUD == nullptr ? Cast<AStarRailHUD>(GetHUD()) : StarRailHUD;
	if (StarRailHUD)
	{
		StarRailHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = StarRailHUD->Announcement &&
			StarRailHUD->Announcement->AnnouncementText &&
			StarRailHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			StarRailHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			StarRailHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AStarRailGameState* StarRailGameState = Cast<AStarRailGameState>(UGameplayStatics::GetGameState(this));
			AStarRailPlayerState* StarRailPlayerState = GetPlayerState<AStarRailPlayerState>();
			if (StarRailGameState && StarRailPlayerState)
			{
				TArray<AStarRailPlayerState*> TopPlayers = StarRailGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(StarRailGameState) : GetInfoText(TopPlayers);

				StarRailHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(GetPawn());
	if (StarRailCharacter && StarRailCharacter->GetCombat())
	{
		StarRailCharacter->bDisableGameplay = true;
		StarRailCharacter->GetCombat()->FireButtonPressed(false);
	}
}

FString AStarRailPlayerController::GetInfoText(const TArray<class AStarRailPlayerState*>& Players)
{
	AStarRailPlayerState* StarRailPlayerState = GetPlayerState<AStarRailPlayerState>();
	if (StarRailPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == StarRailPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString AStarRailPlayerController::GetTeamsInfoText(AStarRailGameState* StarRailGameState)
{
	if (StarRailGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = StarRailGameState->RedTeamScore;
	const int32 BlueTeamScore = StarRailGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}