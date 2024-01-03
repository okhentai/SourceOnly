// Done


#include "CaptureTheFlagGameMode.h"
#include "UEStarRail/Weapon/Flag.h"
#include "UEStarRail/CaptureTheFlag/FlagZone.h"
#include "UEStarRail/GameState/StarRailGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(class AStarRailCharacter* ElimmedCharacter, class AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController)
{
	AStarRailGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AStarRailGameState* BGameState = Cast<AStarRailGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}