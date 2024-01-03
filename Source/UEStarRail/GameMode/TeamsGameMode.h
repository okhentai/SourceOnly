// Done

#pragma once

#include "CoreMinimal.h"
#include "StarRailGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API ATeamsGameMode : public AStarRailGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class AStarRailCharacter* ElimmedCharacter, class AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController) override;
protected:
	virtual void HandleMatchHasStarted() override;
};