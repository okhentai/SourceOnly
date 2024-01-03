// Done

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UEStarRail/StarRailTypes/Team.h"
#include "StarRailPlayerState.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API AStarRailPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
private:
	UPROPERTY()
	class AStarRailCharacter* Character;
	UPROPERTY()
	class AStarRailPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
};