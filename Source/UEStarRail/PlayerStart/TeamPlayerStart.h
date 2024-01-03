// Done

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "UEStarRail/StarRailTypes/Team.h"
#include "TeamPlayerStart.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};