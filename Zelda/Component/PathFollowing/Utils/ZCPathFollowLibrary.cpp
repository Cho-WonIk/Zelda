
#include "ZCPathFollowLibrary.h"
#include "AIController.h"
#include "Component/PathFollowing/ZCPathFollowingComponent.h"
#include "Settings/PathFollow/ZCPathFollowWorldSettings.h"

TArray<FVector> UZCPathFollowLibrary::SmoothNavPath(UObject* WorldContextObject, const TArray<FVector>& PathPoints)
{
	const UZCPathFollowWorldSettings* Settings = GetDefault<UZCPathFollowWorldSettings>();
	if (!WorldContextObject || PathPoints.Num() < 3 || !Settings)
	{
		return PathPoints;
	}

	TArray<FNavPathPoint> NavPathPoints = PointsToNavPathPoints(PathPoints);
	ZCPathSmoothing::SmoothNavPath(NavPathPoints, Settings->SimplificationSettings, Settings->SmoothingSettings);

	return NavPathPointsToPoints(NavPathPoints);
}

TArray<FNavPathPoint> UZCPathFollowLibrary::PointsToNavPathPoints(const TArray<FVector>& PathPoints)
{
	TArray<FNavPathPoint> NavPathPoints;
	for (const FVector& Point : PathPoints)
	{
		NavPathPoints.Add(FNavPathPoint(Point));
	}
	return NavPathPoints;
}

TArray<FVector> UZCPathFollowLibrary::NavPathPointsToPoints(const TArray<FNavPathPoint>& NavPathPoints)
{
	TArray<FVector> PathPoints;
	for (const FNavPathPoint& NavPoint : NavPathPoints)
	{
		PathPoints.Add(NavPoint.Location);
	}
	return PathPoints;
}
