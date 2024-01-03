// Done

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "StarRailGameState.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API AStarRailGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AStarRailPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<AStarRailPlayerState*> TopScoringPlayers;

	/**
	* Teams
	*/

	void RedTeamScores();
	void BlueTeamScores();

	TArray<AStarRailPlayerState*> RedTeam;
	TArray<AStarRailPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

private:

	float TopScore = 0.f;
};