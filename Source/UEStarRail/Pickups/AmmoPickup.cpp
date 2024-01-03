// Done


#include "AmmoPickup.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "UEStarRail/StarRailComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(OtherActor);
	if (StarRailCharacter)
	{
		UCombatComponent* Combat = StarRailCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}