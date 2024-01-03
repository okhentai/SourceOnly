// Done

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AStarRailCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AStarRailCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<AStarRailCharacter*, uint32> BodyShots;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UESTARRAIL_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class AStarRailCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/**
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(
		class AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	/**
	* Projectile
	*/
	FServerSideRewindResult ProjectileServerSideRewind(
		AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/**
	* Shotgun
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AStarRailCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AStarRailCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(AStarRailCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AStarRailCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AStarRailCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AStarRailCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AStarRailCharacter* HitCharacter, float HitTime);

	/**
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/**
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AStarRailCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/**
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

private:

	UPROPERTY()
	AStarRailCharacter* Character;

	UPROPERTY()
	class AStarRailPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:


};