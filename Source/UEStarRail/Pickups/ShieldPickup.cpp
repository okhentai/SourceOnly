// Done


#include "ShieldPickup.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "UEStarRail/StarRailComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(OtherActor);
	if (StarRailCharacter)
	{
		UBuffComponent* Buff = StarRailCharacter->GetBuff();
		if (Buff)
		{
			Buff->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}

	Destroy();
}