// Done

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "StarRailGameMode.generated.h"

namespace MatchState
{
	extern UESTARRAIL_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 *
 */
UCLASS()
class UESTARRAIL_API AStarRailGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AStarRailGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AStarRailCharacter* ElimmedCharacter, class AStarRailPlayerController* VictimController, AStarRailPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class AStarRailPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};