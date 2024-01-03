// Done

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AStarRailCharacter* ElimmedCharacter, class AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};