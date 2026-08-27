
#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavigationTypes.h"
#include "ZCPathFollowUtils.generated.h"

// 경로 단순화 설정
USTRUCT()
struct FZCPathSimplificationSettings
{
	GENERATED_BODY()

	/** 곡선 밀도를 최적화하기 위해 가까운 지점을 동적으로 병합하며 경로 정확도를 유지 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", DisplayName = "병합 허용 거리"), Category = "경로 단순화")
	float MergeTolerance = 75.f;

	/** 경로 단순화를 위한 허용 오차. 값이 낮을수록 더 많은 점을 유지하며, 높을수록 더 과감하게 단순화됨 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", DisplayName = "단순화 허용 오차"), Category = "경로 단순화")
	float SimplificationTolerance = 25.f;
};

// 경로 곡선화 설정
USTRUCT()
struct FZCPathSmoothingSettings
{
	GENERATED_BODY()

	/** Catmull-Rom 보간 시 곡선의 날카로움을 제어. 값이 높을수록 더 날카롭고 뚜렷한 회전을 생성 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "곡선 날카로움(Sharpness)"), Category = "경로 곡선화")
	float CurveSharpness = 0.15f;

	/** 곡선 탄젠트 길이를 제어하여 큰 구간에서 과도한 왜곡을 방지하고 부드러운 이동을 보장 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", DisplayName = "최대 탄젠트 길이"), Category = "경로 곡선화")
	float MaxTangentLength = 450.f;

	/** 구간당 최소 세분화 수. 일정 수준 이상의 부드러움을 보장 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", DisplayName = "세그먼트당 최소 세분화 수"), Category = "경로 곡선화")
	int32 MinSubdivisionsPerSegment = 6;

	/** 추가 세분화를 적용하기 위한 최대 구간 길이. 값이 클수록 긴 구간에서 적은 포인트를 생성 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", DisplayName = "세그먼트 최대 길이"), Category = "경로 곡선화")
	float MaxSegmentSubdivisionLength = 125.f;

	/** 경로를 따라 이동하는 캐릭터의 부드러운 회전 여부 설정 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "캐릭터 회전 부드럽게 처리"), Category = "경로 곡선화")
	bool bSmoothCharacterRotation = true;
};

// Catmull-Rom 곡선 세그먼트 데이터를 저장
USTRUCT()
struct FZCCatmullRomSegmentData
{
	GENERATED_BODY()

	FNavPathPoint StartPoint;
	FNavPathPoint EndPoint;
	FVector2D a, b, c, d;
	float SegmentLength;
};

namespace ZCPathSmoothing
{
	// 경로를 단순화 및 곡선화
	void SmoothNavPath(TArray<FNavPathPoint>& PathPoints, const FZCPathSimplificationSettings& SimplificationSettings, const FZCPathSmoothingSettings& SmoothingSettings);

#if !UE_BUILD_SHIPPING
	// 경로를 시각적으로 디버그 표시
	void DrawPath(const class UWorld* World, const TArray<FNavPathPoint>& PathPoints, const FColor& PathColor, float LineThickness = 10.f);
#endif // !UE_BUILD_SHIPPING

	namespace Internal
	{
		// NavLink 기준으로 경로를 분할
		TArray<TArray<FNavPathPoint>> SplitPathByNavLinks(const TArray<FNavPathPoint>& PathPoints);

		// 분할된 경로들을 다시 병합
		TArray<FNavPathPoint> MergeNavPaths(const TArray<TArray<FNavPathPoint>>& SubPaths);

		// 가까운 경로 지점을 병합
		void MergeClosestPathPoints(TArray<FNavPathPoint>& PathPoints, float Tolerance);

		// 경로 지점을 단순화
		void SimplifyPathPoints(TArray<FNavPathPoint>& PathPoints, float Tolerance);

		// Catmull-Rom 보간을 통해 경로를 곡선화
		void SmoothPathWithCatmullRom(TArray<FNavPathPoint>& PathPoints, const FZCPathSmoothingSettings& SmoothingSettings);

		// Catmull-Rom 세그먼트 데이터 사전 계산
		TArray<FZCCatmullRomSegmentData> PrecalculateCatmullRomSegments(const TArray<FNavPathPoint>& PathPoints, float Tension, float MaxTangentLength);

		// 경계에 가상 포인트 추가
		void AddBoundaryVirtualPoints(TArray<FNavPathPoint>& PathPoints);

		// 2D Catmull-Rom 보간 계산
		FVector2D CatmullRomCentripetalInterpolation2D(const FZCCatmullRomSegmentData& SegmentData, float t);

		// 두 직선의 교차점을 계산
		bool FindIntersectionPoint2D(const FVector2D& A, const FVector2D& Dir1, const FVector2D& B, const FVector2D& Dir2, FVector2D& OutIntersection);

		// FVector 배열을 NavPathPoint 배열로 변환
		TArray<FNavPathPoint> PointsToNavPathPoints(const TArray<FVector>& PathPoints);

		// NavPathPoint 배열을 FVector 배열로 변환
		TArray<FVector> NavPathPointsToPoints(const TArray<FNavPathPoint>& NavPathPoints);
	}
}
