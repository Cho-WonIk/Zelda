
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZCPathFollowUtils.h"
#include "ZCPathFollowLibrary.generated.h"

UCLASS()
class ZELDA_API UZCPathFollowLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* 내비게이션 경로 곡선화
	* Catmull-Rom 스플라인을 사용하여 주어진 내비게이션 경로를 곡선으로 변환
	* 많아진 포인트들에서 일부 불필요한 포인트들을 제거
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Controller"), Category = "AI|Navigation")
	static TArray<FVector> SmoothNavPath(UObject* WorldContextObject, const TArray<FVector>& PathPoints);

private:
	static TArray<FNavPathPoint> PointsToNavPathPoints(const TArray<FVector>& PathPoints);
	static TArray<FVector> NavPathPointsToPoints(const TArray<FNavPathPoint>& NavPathPoints);

};