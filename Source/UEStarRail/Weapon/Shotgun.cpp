// Done


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UEStarRail/Character/StarRailCharacter.h"
#include "UEStarRail/PlayerController/StarRailPlayerController.h"
#include "UEStarRail/StarRailComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// Maps hit character to number of times hit
		TMap<AStarRailCharacter*, uint32> HitMap;
		TMap<AStarRailCharacter*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AStarRailCharacter* StarRailCharacter = Cast<AStarRailCharacter>(FireHit.GetActor());
			if (StarRailCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(StarRailCharacter)) HeadShotHitMap[StarRailCharacter]++;
					else HeadShotHitMap.Emplace(StarRailCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(StarRailCharacter)) HitMap[StarRailCharacter]++;
					else HitMap.Emplace(StarRailCharacter, 1);
				}


				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}
		}
		TArray<AStarRailCharacter*> HitCharacters;

		// Maps Character hit to total damage
		TMap<AStarRailCharacter*, float> DamageMap;

		// Calculate body shot damage by multiplying times hit x Damage - store in DamageMap
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);

				HitCharacters.AddUnique(HitPair.Key);
			}
		}

		// Calculate head shot damage by multiplying times hit x HeadShotDamage - store in DamageMap
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		// Loop through DamageMap to get total damage for each character
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, // Character that was hit
						DamagePair.Value, // Damage calculated in the two for loops above
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}


		if (!HasAuthority() && bUseServerSideRewind)
		{
			StarRailOwnerCharacter = StarRailOwnerCharacter == nullptr ? Cast<AStarRailCharacter>(OwnerPawn) : StarRailOwnerCharacter;
			StarRailOwnerController = StarRailOwnerController == nullptr ? Cast<AStarRailPlayerController>(InstigatorController) : StarRailOwnerController;
			if (StarRailOwnerController && StarRailOwnerCharacter && StarRailOwnerCharacter->GetLagCompensation() && StarRailOwnerCharacter->IsLocallyControlled())
			{
				StarRailOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					StarRailOwnerController->GetServerTime() - StarRailOwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}