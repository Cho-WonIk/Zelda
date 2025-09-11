// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Spline/ZCPatrolSplineActor.h"
#include "Zelda/Physics/ZCCollision.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

AZCPatrolSplineActor::AZCPatrolSplineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(SplineComponent); // Spline에 붙임
	ArrowComponent->ArrowSize = 2.0f;
	ArrowComponent->SetHiddenInGame(true); // 게임에선 숨김

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionProfileName(Zelda::Profile::InteractionTrigger);
	SphereComponent->SetSphereRadius(10.0f);
	SphereComponent->SetupAttachment(ArrowComponent); // Spline에 붙임
	SphereComponent->CanCharacterStepUpOn = ECB_No; // 캐릭터가 스플라인 위로 올라가지 않도록 설정
}

#if WITH_EDITOR
void AZCPatrolSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AZCPatrolSplineActor, NumPoints))
	{
		GenerateSplinePoints();
	}

	UpdateStartArrow();
}
#endif

void AZCPatrolSplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	GenerateSplinePoints();

#if WITH_EDITOR
	UpdateStartArrow();
#endif
}

void AZCPatrolSplineActor::GenerateSplinePoints()
{
	SplineComponent->ClearSplinePoints(false);

	const float Radius = 300.0f;
	for (int32 i = 0; i < NumPoints; ++i)
	{
		float Angle = FMath::DegreesToRadians(360.0f / NumPoints * i);
		FVector PointLocation = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

		FSplinePoint Point;
		Point.InputKey = i;
		Point.Position = PointLocation;
		Point.Type = ESplinePointType::Curve;

		SplineComponent->AddPoint(Point, false);
	}

	SplineComponent->SetClosedLoop(true, false);
	SplineComponent->UpdateSpline();
}

#if WITH_EDITOR
void AZCPatrolSplineActor::UpdateStartArrow()
{

	if (!GetWorld() || GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	if (!SplineComponent || SplineComponent->GetNumberOfSplinePoints() < 2)
	{
		ArrowComponent->SetVisibility(false);
		return;
	}

	FVector Start = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	FVector Next = SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
	FVector Direction = (Next - Start).GetSafeNormal();

	ArrowComponent->SetWorldLocation(Start);
	ArrowComponent->SetWorldRotation(Direction.Rotation());
	ArrowComponent->SetVisibility(true);
}
#endif
