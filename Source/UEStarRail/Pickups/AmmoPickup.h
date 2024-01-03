// Done

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "UEStarRail/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 *
 */
UCLASS()
class UESTARRAIL_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};