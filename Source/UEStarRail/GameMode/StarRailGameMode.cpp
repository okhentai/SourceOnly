// Done


#include "StarRailGameMode.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "UEStarRail/PlayerController/StarRailPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "UEStarRail/PlayerState/StarRailPlayerState.h"
#include "UEStarRail/GameState/StarRailGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AStarRailGameMode::AStarRailGameMode()
{
	bDelayedStart = true;
}

void AStarRailGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AStarRailGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AStarRailGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AStarRailPlayerController* StarRailPlayer = Cast<AStarRailPlayerController>(*It);
		if (StarRailPlayer)
		{
			StarRailPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float AStarRailGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AStarRailGameMode::PlayerEliminated(class AStarRailCharacter* ElimmedCharacter, class AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AStarRailPlayerState* AttackerPlayerState = AttackerController ? Cast<AStarRailPlayerState>(AttackerController->PlayerState) : nullptr;
	AStarRailPlayerState* VictimPlayerState = VictimController ? Cast<AStarRailPlayerState>(VictimController->PlayerState) : nullptr;

	AStarRailGameState* StarRailGameState = GetGameState<AStarRailGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && StarRailGameState)
	{
		TArray<AStarRailPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : StarRailGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);
		StarRailGameState->UpdateTopScore(AttackerPlayerState);
		if (StarRailGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AStarRailCharacter* Leader = Cast<AStarRailCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!StarRailGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AStarRailCharacter* Loser = Cast<AStarRailCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AStarRailPlayerController* StarRailPlayer = Cast<AStarRailPlayerController>(*It);
		if (StarRailPlayer && AttackerPlayerState && VictimPlayerState)
		{
			StarRailPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AStarRailGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void AStarRailGameMode::PlayerLeftGame(AStarRailPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AStarRailGameState* StarRailGameState = GetGameState<AStarRailGameState>();
	if (StarRailGameState && StarRailGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		StarRailGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AStarRailCharacter* CharacterLeaving = Cast<AStarRailCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}