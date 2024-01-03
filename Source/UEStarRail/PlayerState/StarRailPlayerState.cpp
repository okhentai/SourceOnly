// Done


#include "StarRailPlayerState.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "UEStarRail/PlayerController/StarRailPlayerController.h"
#include "Net/UnrealNetwork.h"

void AStarRailPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStarRailPlayerState, Defeats);
	DOREPLIFETIME(AStarRailPlayerState, Team);
}

void AStarRailPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AStarRailCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AStarRailPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AStarRailPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AStarRailCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AStarRailPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AStarRailPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AStarRailCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AStarRailPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AStarRailPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AStarRailCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AStarRailPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AStarRailPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	AStarRailCharacter* BCharacter = Cast <AStarRailCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void AStarRailPlayerState::OnRep_Team()
{
	AStarRailCharacter* BCharacter = Cast <AStarRailCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}