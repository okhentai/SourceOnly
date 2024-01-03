// Done


#include "StarRailGameState.h"
#include "Net/UnrealNetwork.h"
#include "UEStarRail/PlayerState/StarRailPlayerState.h"
#include "UEStarRail/PlayerController/StarRailPlayerController.h"

void AStarRailGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStarRailGameState, TopScoringPlayers);
	DOREPLIFETIME(AStarRailGameState, RedTeamScore);
	DOREPLIFETIME(AStarRailGameState, BlueTeamScore);
}

void AStarRailGameState::UpdateTopScore(class AStarRailPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AStarRailGameState::RedTeamScores()
{
	++RedTeamScore;
	AStarRailPlayerController* BPlayer = Cast<AStarRailPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AStarRailGameState::BlueTeamScores()
{
	++BlueTeamScore;
	AStarRailPlayerController* BPlayer = Cast<AStarRailPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AStarRailGameState::OnRep_RedTeamScore()
{
	AStarRailPlayerController* BPlayer = Cast<AStarRailPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AStarRailGameState::OnRep_BlueTeamScore()
{
	AStarRailPlayerController* BPlayer = Cast<AStarRailPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}