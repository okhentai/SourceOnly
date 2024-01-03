// Done


#include "TeamsGameMode.h"
#include "UEStarRail/GameState/StarRailGameState.h"
#include "UEStarRail/PlayerState/StarRailPlayerState.h"
#include "UEStarRail/PlayerController/StarRailPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AStarRailGameState* BGameState = Cast<AStarRailGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		AStarRailPlayerState* BPState = NewPlayer->GetPlayerState<AStarRailPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AStarRailGameState* BGameState = Cast<AStarRailGameState>(UGameplayStatics::GetGameState(this));
	AStarRailPlayerState* BPState = Exiting->GetPlayerState<AStarRailPlayerState>();
	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}

}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AStarRailGameState* BGameState = Cast<AStarRailGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			AStarRailPlayerState* BPState = Cast<AStarRailPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AStarRailPlayerState* AttackerPState = Attacker->GetPlayerState<AStarRailPlayerState>();
	AStarRailPlayerState* VictimPState = Victim->GetPlayerState<AStarRailPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AStarRailCharacter* ElimmedCharacter, AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AStarRailGameState* BGameState = Cast<AStarRailGameState>(UGameplayStatics::GetGameState(this));
	AStarRailPlayerState* AttackerPlayerState = AttackerController ? Cast<AStarRailPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}