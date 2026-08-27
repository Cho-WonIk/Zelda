// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "Damage/ZCDamage.h"
#include "GameData/Table/ChemistrySystemTable.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Physics/ZCShape.h"

#include "Component/Chemistry/ZCMaterialStateComponent.h"

#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::Gameplay
{
	static bool bDrawDebugAll = false;
	static bool bDrawShape = false;

	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::Gameplay::all, bDrawDebugAll, TEXT("Gameplay 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugPlayer(Zelda::Debug::Gameplay::shape, bDrawShape, TEXT("Gameplay - 히트 콜리전 시각화"), ECVF_Default);
}
#endif

// GameplayStatics 내부 헬퍼 함수
/** @RETURN True if weapon trace from Origin hits component VictimComp.  OutHitResult will contain properties of the hit. */
static bool ComponentIsDamageableFrom(UPrimitiveComponent* VictimComp, FVector const& Origin, AActor const* IgnoredActor, const TArray<AActor*>& IgnoreActors, ECollisionChannel TraceChannel, FHitResult& OutHitResult)
{
	FCollisionQueryParams LineParams(SCENE_QUERY_STAT(ComponentIsVisibleFrom), true, IgnoredActor);
	LineParams.AddIgnoredActors(IgnoreActors);

	// Do a trace from origin to middle of box
	UWorld* const World = VictimComp->GetWorld();
	check(World);

	FVector const TraceEnd = VictimComp->Bounds.Origin;
	FVector TraceStart = Origin;
	if (Origin == TraceEnd)
	{
		// tiny nudge so LineTraceSingle doesn't early out with no hits
		TraceStart.Z += 0.01f;
	}

	// Only do a line trace if there is a valid channel, if it is invalid then result will have no fall off
	if (TraceChannel != ECollisionChannel::ECC_MAX)
	{
		bool const bHadBlockingHit = World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, LineParams);
		//::DrawDebugLine(World, TraceStart, TraceEnd, FLinearColor::Red, true);

		// If there was a blocking hit, it will be the last one
		if (bHadBlockingHit)
		{
			if (OutHitResult.Component == VictimComp)
			{
				// if blocking hit was the victim component, it is visible
				return true;
			}
			else
			{
				// if we hit something else blocking, it's not
				UE_LOG(LogDamage, Log, TEXT("Radial Damage to %s blocked by %s (%s)"), *GetNameSafe(VictimComp), *OutHitResult.GetHitObjectHandle().GetName(), *GetNameSafe(OutHitResult.Component.Get()));
				return false;
			}
		}
	}
	else
	{
		UE_LOG(LogDamage, Warning, TEXT("ECollisionChannel::ECC_MAX is not valid! No falloff is added to damage"));
	}

	// didn't hit anything, assume nothing blocking the damage and victim is consequently visible
	// but since we don't have a hit result to pass back, construct a simple one, modeling the damage as having hit a point at the component's center.
	FVector const FakeHitLoc = VictimComp->GetComponentLocation();
	FVector const FakeHitNorm = (Origin - FakeHitLoc).GetSafeNormal();		// normal points back toward the epicenter
	OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp, FakeHitLoc, FakeHitNorm);
	return true;
}

float UZCGameplayFunctionLibrary::ApplyDamage(FElementInfo& ElementInfo, AActor* DamagedActor, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UZCDamageType> DamageTypeClass)
{
	if (DamagedActor)// && (BaseDamage != 0.f))
	{
		TSubclassOf<UZCDamageType> const ValidDamageType = DamageTypeClass ? DamageTypeClass : TSubclassOf<UZCDamageType>(UZCDamageType::StaticClass());
		FZCDamageEvent DamageEvent(ValidDamageType, ElementInfo.ElementTag, ElementInfo.Duration, ElementInfo.SpreadCount);

		return DamagedActor->TakeDamage(BaseDamage, DamageEvent, EventInstigator, DamageCauser);
	}

	return 0.0f;
}

float UZCGameplayFunctionLibrary::ApplyPointDamage(FElementInfo& ElementInfo, AActor* DamagedActor, float BaseDamage, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UZCDamageType> DamageTypeClass)
{
	if (DamagedActor && BaseDamage != 0.f)
	{
		TSubclassOf<UZCDamageType> const ValidDamageType = DamageTypeClass ? DamageTypeClass : TSubclassOf<UZCDamageType>(UZCDamageType::StaticClass());
		FZCPointDamageEvent PointDamageEvent(BaseDamage, HitInfo, HitFromDirection, ValidDamageType, ElementInfo.ElementTag, ElementInfo.Duration, ElementInfo.SpreadCount);

		return DamagedActor->TakeDamage(BaseDamage, PointDamageEvent, EventInstigator, DamageCauser);
	}

	return 0.0f;
}

bool UZCGameplayFunctionLibrary::ApplyRadialDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FVector& Origin, float DamageRadius, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, bool bDoFullDamage, ECollisionChannel DamagePreventionChannel)
{
	float DamageFalloff = bDoFullDamage ? 0.f : 1.f;
	return ApplyRadialDamageWithFalloff(ElementInfo, WorldContextObject, BaseDamage, 0.f, Origin, 0.f, DamageRadius, DamageFalloff, DamageTypeClass, IgnoreActors, DamageCauser, InstigatedByController, DamagePreventionChannel);
}

bool UZCGameplayFunctionLibrary::ApplyRadialDamageWithFalloff(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel)
{
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage), false, DamageCauser);

	SphereParams.AddIgnoredActors(IgnoreActors);

	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
#if !UE_BUILD_SHIPPING
		using namespace Zelda::Debug::Gameplay;

		if (bDrawDebugAll || bDrawShape)
		{
			const FColor InnerColor = FColor::Green;
			const FColor OuterColor = FColor::Red;

			DrawDebugSphere(World, Origin, DamageInnerRadius, 16, InnerColor, false, 0.125f, 0, 1.f);
			DrawDebugSphere(World, Origin, DamageOuterRadius, 16, OuterColor, false, 0.125f, 0, 1.f);

		}
#endif //!UE_BUILD_SHIPPING
		World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, DamagePreventionChannel, FCollisionShape::MakeSphere(DamageOuterRadius), SphereParams);
	}

	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* const OverlapActor = Overlap.OverlapObjectHandle.FetchActor();

		if (OverlapActor && OverlapActor->CanBeDamaged() && OverlapActor != DamageCauser && Overlap.Component.IsValid())
		{
			FHitResult Hit;
			if (ComponentIsDamageableFrom(Overlap.Component.Get(), Origin, DamageCauser, IgnoreActors, DamagePreventionChannel, Hit))
			{
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}

	bool bAppliedDamage = false;

	if (OverlapComponentMap.Num() > 0)
	{
		TSubclassOf<UZCDamageType> const ValidDamageTypeClass = DamageTypeClass ? DamageTypeClass : TSubclassOf<UZCDamageType>(UZCDamageType::StaticClass());

		FZCRadialDamageEvent DmgEvent;

		DmgEvent.ElementTag = ElementInfo.ElementTag;
		DmgEvent.ElementDuration = ElementInfo.Duration;
		DmgEvent.ElementSpreadingCount = ElementInfo.SpreadCount;

		DmgEvent.DamageTypeClass = ValidDamageTypeClass;
		DmgEvent.Origin = Origin;
		DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, DamageFalloff);

		for (const auto& It : OverlapComponentMap)
		{
			AActor* const Victim = It.Key;
			TArray<FHitResult> const& ComponentHits = It.Value;
			DmgEvent.ComponentHits = ComponentHits;

			Victim->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);

			bAppliedDamage = true;
		}

	}

	return bAppliedDamage;
}

bool UZCGameplayFunctionLibrary::ApplyShapeDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FTransform& OwnerTransform, const FZCShape& ZCDamageShape, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel)
{
	// 위치/회전 추출
	FVector Origin;
	FQuat   Rotation;
	FCollisionShape DamageShape;
	ZCDamageShape.BuildWorldQuery(OwnerTransform, Origin, Rotation, DamageShape);

	FCollisionQueryParams ShapeParams(SCENE_QUERY_STAT(ApplyShapeDamage), false, DamageCauser);
	ShapeParams.AddIgnoredActors(IgnoreActors);

	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
#if !UE_BUILD_SHIPPING
		using namespace Zelda::Debug::Gameplay;

		if (bDrawDebugAll || bDrawShape)
		{
			Zelda::DrawDebugShape(World, ZCDamageShape, OwnerTransform, FColor::Red, 1.5f, 0.25f, 0, false, true, 6.0f);
		}

#endif //!UE_BUILD_SHIPPING
		World->OverlapMultiByChannel(Overlaps, Origin, Rotation, DamagePreventionChannel, DamageShape, ShapeParams);
	}

	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* const OverlapActor = Overlap.OverlapObjectHandle.FetchActor();

		if (OverlapActor && OverlapActor->CanBeDamaged() && OverlapActor != DamageCauser && Overlap.Component.IsValid())
		{
			FHitResult Hit;
			if (ComponentIsDamageableFrom(Overlap.Component.Get(), Origin, DamageCauser, IgnoreActors, DamagePreventionChannel, Hit))
			{
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}

	bool bAppliedDamage = false;

	if (OverlapComponentMap.Num() > 0)
	{
		TSubclassOf<UZCDamageType> const ValidDamageTypeClass = DamageTypeClass ? DamageTypeClass : TSubclassOf<UZCDamageType>(UZCDamageType::StaticClass());

		FZCRadialDamageEvent DmgEvent;

		DmgEvent.ElementTag = ElementInfo.ElementTag;
		DmgEvent.ElementDuration = ElementInfo.Duration;
		DmgEvent.ElementSpreadingCount = ElementInfo.SpreadCount;

		DmgEvent.DamageTypeClass = ValidDamageTypeClass;
		DmgEvent.Origin = Origin;

		for (const auto &It : OverlapComponentMap)
		{
			AActor* const Victim = It.Key;
			TArray<FHitResult> const& ComponentHits = It.Value;
			DmgEvent.ComponentHits = ComponentHits;

			Victim->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);

			bAppliedDamage = true;
		}
	}

	return bAppliedDamage;
}


bool UZCGameplayFunctionLibrary::ApplyTouchDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FVector& Start, const FVector& Direction, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ApplyLineDamage), false, DamageCauser);
	TraceParams.AddIgnoredActors(IgnoreActors);

	FHitResult HitResult;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return false;

	const float TouchTraceLength = 40.0f;
	const FVector End = Start + Direction * TouchTraceLength;

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Gameplay;

	if (bDrawDebugAll)
	{
		DrawDebugLine(World, Start, End, FColor::Red, false, 1.5f, 0, 10.f);
	}

#endif // !UE_BUILD_SHIPPING

	const bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, DamagePreventionChannel, TraceParams);

	if (!bHit) return false;

	AActor* const HitActor = HitResult.GetActor();

	if (!HitActor || !HitActor->CanBeDamaged() || HitActor == DamageCauser) return false;

	// 데미지 타입 유효성 보정

	TSubclassOf<UZCDamageType> const ValidDamageType = DamageTypeClass ? DamageTypeClass : TSubclassOf<UZCDamageType>(UZCDamageType::StaticClass());
	FZCPointDamageEvent PointDamageEvent(BaseDamage, HitResult, Direction, ValidDamageType, ElementInfo.ElementTag, ElementInfo.Duration, ElementInfo.SpreadCount);

	HitActor->TakeDamage(BaseDamage, PointDamageEvent, InstigatedByController, DamageCauser);

	return true;
}
