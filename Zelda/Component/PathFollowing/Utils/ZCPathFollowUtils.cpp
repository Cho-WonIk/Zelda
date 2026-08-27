
#include "ZCPathFollowUtils.h"
#include "NavMesh/RecastNavMesh.h"
#include "DrawDebugHelpers.h"

namespace ZCPathSmoothing
{
    void SmoothNavPath(TArray<FNavPathPoint>& PathPoints, const FZCPathSimplificationSettings& SimplificationSettings, const FZCPathSmoothingSettings& SmoothingSettings)
    {
        // Nav 링크에 따라 경로를 여러 하위 경로로 분할
        TArray<TArray<FNavPathPoint>> SubPaths = Internal::SplitPathByNavLinks(PathPoints);

        // 각 하위 경로에 대해 처리 수행
        for (TArray<FNavPathPoint>& SubPath : SubPaths)
        {
            // 1단계: 가장 가까운 포인트 병합
            Internal::MergeClosestPathPoints(SubPath, SimplificationSettings.MergeTolerance);

            // 2단계: 경로 단순화
            Internal::SimplifyPathPoints(SubPath, SimplificationSettings.SimplificationTolerance);

            // 3단계: Catmull-Rom 곡선화 적용
            Internal::SmoothPathWithCatmullRom(SubPath, SmoothingSettings);
        }

        // 처리된 모든 하위 경로를 하나의 경로로 병합
        PathPoints = Internal::MergeNavPaths(SubPaths);
    }

#if !UE_BUILD_SHIPPING
    void DrawPath(const UWorld* World, const TArray<FNavPathPoint>& PathPoints, const FColor& PathColor, float LineThickness)
    {
        if (World == nullptr || PathPoints.Num() < 2) return;

        for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
        {
            if (!FNavMeshNodeFlags(PathPoints[i].Flags).IsNavLink())
            {
                const FVector Start = PathPoints[i].Location + FVector(0.0, 0.0, LineThickness);
                const FVector End = PathPoints[i + 1].Location + FVector(0.0, 0.0, LineThickness);

                DrawDebugLine(World, Start, End, PathColor, false, -1.0f, 0, LineThickness);
            }
        }
    }
#endif

    namespace Internal
    {
        TArray<TArray<FNavPathPoint>> SplitPathByNavLinks(const TArray<FNavPathPoint>& PathPoints)
        {
            TArray<TArray<FNavPathPoint>> SubPaths;
            TArray<FNavPathPoint> CurrentSubPath;

            for (int32 i = 0; i < PathPoints.Num(); i++)
            {
                const FNavPathPoint& Point = PathPoints[i];
                CurrentSubPath.Add(Point);

                // CustomNavLinkId 또는 Legacy NavLink 여부 확인
                if (Point.CustomNavLinkId != FNavLinkId::Invalid || FNavMeshNodeFlags(Point.Flags).IsNavLink())
                {
                    SubPaths.Add(CurrentSubPath);
                    CurrentSubPath.Empty();
                }
            }

            // 마지막 하위 경로 추가
            if (CurrentSubPath.Num() > 0)
            {
                SubPaths.Add(CurrentSubPath);
            }

            return SubPaths;
        }

        TArray<FNavPathPoint> MergeNavPaths(const TArray<TArray<FNavPathPoint>>& SubPaths)
        {
            TArray<FNavPathPoint> MergedPath;

            for (const TArray<FNavPathPoint>& SubPath : SubPaths)
            {
                MergedPath.Append(SubPath);
            }

            return MergedPath;
        }

        void MergeClosestPathPoints(TArray<FNavPathPoint>& PathPoints, float Tolerance)
        {
            if (PathPoints.Num() < 4) return;

            const float ToleranceSquared = FMath::Square(Tolerance);
            TArray<FNavPathPoint> MergedPath;

            MergedPath.Add(PathPoints[0]);
            MergedPath.Add(PathPoints[1]);

            for (int32 i = 2; i < PathPoints.Num() - 1; ++i)
            {
                const FVector2D P1 = FVector2D(MergedPath[MergedPath.Num() - 2].Location);
                const FVector2D P2 = FVector2D(MergedPath.Last().Location);
                const FVector2D P3 = FVector2D(PathPoints[i].Location);
                const FVector2D P4 = FVector2D(PathPoints[i + 1].Location);

                // 구간의 방향 계산
                const FVector2D Dir1 = (P2 - P1).GetSafeNormal();
                const FVector2D Dir2 = (P4 - P3).GetSafeNormal();

                // 정렬 정도 계산
                const float Alignment = FMath::Abs(FVector2D::DotProduct(Dir1, Dir2));
                if (Alignment > 0.85f)
                {
                    // 정렬된 경우 병합 생략
                    MergedPath.Add(PathPoints[i]);
                    continue;
                }

                // 구간 사이 거리 계산 (P2 -> P3)
                const float DistanceSquared = FVector2D::DistSquared(P2, P3);

                if (DistanceSquared <= ToleranceSquared)
                {
                    // 교차점 계산 후 병합
                    FVector2D IntersectionPoint;
                    if (FindIntersectionPoint2D(P1, Dir1, P3, Dir2, IntersectionPoint))
                    {
                        // 교차점과 중점 사이에서 보간
                        const FVector2D SegmentMidpoint = (P2 + P3) * 0.5f;
                        const FVector2D ModifiedIntersection = FMath::Lerp(IntersectionPoint, SegmentMidpoint, 0.5f);

                        FNavPathPoint MergedPoint = PathPoints[i];
                        MergedPoint.Location = FVector(ModifiedIntersection, MergedPoint.Location.Z);
                        MergedPath.Last() = MergedPoint;
                    }
                    else
                    {
                        // 실패 시 중점을 사용
                        const FVector2D FallbackPoint = (P2 + P3) * 0.5f;
                        FNavPathPoint MergedPoint = PathPoints[i];
                        MergedPoint.Location = FVector(FallbackPoint, MergedPoint.Location.Z);
                        MergedPath.Last() = MergedPoint;
                    }
                }
                else
                {
                    // 병합하지 않고 유지
                    MergedPath.Add(PathPoints[i]);
                }
            }

            MergedPath.Add(PathPoints.Last());
            PathPoints = MoveTemp(MergedPath);
        }

        void SimplifyPathPoints(TArray<FNavPathPoint>& PathPoints, float Tolerance)
        {
            if (PathPoints.Num() < 3) return;

            const float ToleranceSquared = FMath::Square(Tolerance);

            TArray<FNavPathPoint> SimplifiedPoints;
            SimplifiedPoints.Add(PathPoints[0]);

            for (int32 i = 1; i < PathPoints.Num() - 1; i++)
            {
                // Z 차이가 허용 오차를 넘으면 유지
                if (SimplifiedPoints.Last().Location.Z - PathPoints[i].Location.Z > Tolerance)
                {
                    SimplifiedPoints.Add(PathPoints[i]);
                    continue;
                }

                const FVector2D PreviousPoint = FVector2D(SimplifiedPoints.Last().Location);
                const FVector2D CurrentPoint = FVector2D(PathPoints[i].Location);
                const FVector2D NextPoint = FVector2D(PathPoints[i + 1].Location);

                // 현재 점을 직선 위에 투영
                const FVector2D ClosestPoint = FMath::ClosestPointOnSegment2D(CurrentPoint, PreviousPoint, NextPoint);
                const float DistanceSquared = (CurrentPoint - ClosestPoint).SizeSquared();

                // 오차보다 멀면 유지
                if (DistanceSquared > ToleranceSquared)
                {
                    SimplifiedPoints.Add(PathPoints[i]);
                }
            }
            SimplifiedPoints.Add(PathPoints.Last());

            PathPoints = MoveTemp(SimplifiedPoints);
        }

        void SmoothPathWithCatmullRom(TArray<FNavPathPoint>& PathPoints, const FZCPathSmoothingSettings& SmoothingSettings)
        {
            if (PathPoints.Num() < 3) return;

            // Catmull-Rom 세그먼트 데이터를 미리 계산
            const TArray<FZCCatmullRomSegmentData> SegmentDataList = PrecalculateCatmullRomSegments(PathPoints, SmoothingSettings.CurveSharpness, SmoothingSettings.MaxTangentLength);
            TArray<FNavPathPoint> SmoothedPoints;

            for (const FZCCatmullRomSegmentData& SegmentData : SegmentDataList)
            {
                // 세그먼트 길이를 기반으로 세분화 개수 계산
                const int32 NumSubdivisions = FMath::Max(SmoothingSettings.MinSubdivisionsPerSegment, FMath::CeilToInt(SegmentData.SegmentLength / SmoothingSettings.MaxSegmentSubdivisionLength));
                const float PointDensity = 1.f / NumSubdivisions;

                for (int32 i = 0; i < NumSubdivisions; ++i)
                {
                    // 선형 분포
                    const float LinearT = i * PointDensity;

                    // 비선형 분포 (코사인 기반)
                    const float NonLinearT = 0.5f - 0.5f * FMath::Cos(LinearT * PI);

                    // 선형과 비선형 사이 보간
                    const float CurvedT = FMath::Lerp(LinearT, NonLinearT, 0.5);

                    const FVector2D InterpolatedLocation2D = CatmullRomCentripetalInterpolation2D(SegmentData, CurvedT);
                    FNavPathPoint InterpolatedPoint = SegmentData.StartPoint;
                    InterpolatedPoint.Location.X = InterpolatedLocation2D.X;
                    InterpolatedPoint.Location.Y = InterpolatedLocation2D.Y;

                    // Z축은 선형 보간
                    InterpolatedPoint.Location.Z = FMath::Lerp(SegmentData.StartPoint.Location.Z, SegmentData.EndPoint.Location.Z, CurvedT);

                    SmoothedPoints.Add(InterpolatedPoint);
                }
            }

            SmoothedPoints.Add(PathPoints.Last());
            PathPoints = MoveTemp(SmoothedPoints);
        }

        TArray<FZCCatmullRomSegmentData> PrecalculateCatmullRomSegments(const TArray<FNavPathPoint>& PathPoints, float Tension, float MaxTangentLength)
        {
            TArray<FZCCatmullRomSegmentData> SegmentDataList;
            TArray<FNavPathPoint> VirtualPathPoints = PathPoints;

            // 경계 가상 포인트 추가
            AddBoundaryVirtualPoints(VirtualPathPoints);

            const float Alpha = 0.5f; // 중심 구속 방식
            const float MaxTangentLengthSquared = FMath::Square(MaxTangentLength);

            // 컨트롤 포인트들 사이 거리 기반으로 t 값 계산
            TArray<float> tValues;
            TArray<float> tDistances;

            for (int32 i = 0; i < VirtualPathPoints.Num() - 1; ++i)
            {
                const float distance = FVector2D::Distance(FVector2D(VirtualPathPoints[i].Location), FVector2D(VirtualPathPoints[i + 1].Location));
                tDistances.Add(distance);
                tValues.Add(FMath::Pow(distance, Alpha));
            }

            // 각 세그먼트 계산
            for (int32 i = 1; i < VirtualPathPoints.Num() - 2; ++i)
            {
                FZCCatmullRomSegmentData SegmentData;
                SegmentData.StartPoint = VirtualPathPoints[i];
                SegmentData.EndPoint = VirtualPathPoints[i + 1];

                FVector2D P0 = FVector2D(VirtualPathPoints[i - 1].Location);
                FVector2D P1 = FVector2D(VirtualPathPoints[i].Location);
                FVector2D P2 = FVector2D(VirtualPathPoints[i + 1].Location);
                FVector2D P3 = FVector2D(VirtualPathPoints[i + 2].Location);

                float t01 = tValues[i - 1];
                float t12 = tValues[i];
                float t23 = tValues[i + 1];

                // 탄젠트 m1, m2 계산
                FVector2D m1 = (1.0f - Tension) *
                    (P2 - P1 + t12 * ((P1 - P0) / t01 - (P2 - P0) / (t01 + t12)));

                FVector2D m2 = (1.0f - Tension) *
                    (P2 - P1 + t12 * ((P3 - P2) / t23 - (P3 - P1) / (t12 + t23)));

                // 탄젠트 길이 제한
                if (m1.SizeSquared() > MaxTangentLengthSquared)
                {
                    m1 = m1.GetSafeNormal() * MaxTangentLength;
                }

                if (m2.SizeSquared() > MaxTangentLengthSquared)
                {
                    m2 = m2.GetSafeNormal() * MaxTangentLength;
                }

                // 보간식 계수 a, b, c, d 계산
                SegmentData.a = 2.0f * (P1 - P2) + m1 + m2;
                SegmentData.b = -3.0f * (P1 - P2) - m1 - m1 - m2;
                SegmentData.c = m1;
                SegmentData.d = P1;

                SegmentData.SegmentLength = tDistances[i];

                SegmentDataList.Add(SegmentData);
            }

            return SegmentDataList;
        }

        void AddBoundaryVirtualPoints(TArray<FNavPathPoint>& PathPoints)
        {
            // 경계 지점 외삽을 통해 가상 포인트 생성 (P0, P3 역할)
            const FVector2D FirstVirtual = FVector2D(PathPoints[0].Location) + (FVector2D(PathPoints[0].Location) - FVector2D(PathPoints[1].Location)) * 0.5f;
            const FVector2D LastVirtual = FVector2D(PathPoints.Last().Location) + (FVector2D(PathPoints.Last().Location) - FVector2D(PathPoints[PathPoints.Num() - 2].Location)) * 0.5f;

            // 가상 포인트를 NavPathPoint로 변환
            const FNavPathPoint VirtualStart(FVector(FirstVirtual, PathPoints[0].Location.Z));
            const FNavPathPoint VirtualEnd(FVector(LastVirtual, PathPoints.Last().Location.Z));

            // 앞뒤로 삽입
            PathPoints.Insert(VirtualStart, 0);
            PathPoints.Add(VirtualEnd);
        }

        FVector2D CatmullRomCentripetalInterpolation2D(const FZCCatmullRomSegmentData& SegmentData, float t)
        {
            t = FMath::Clamp(t, 0.0f, 1.0f);
            float tSquared = t * t;
            float tCubed = tSquared * t;

            // 사전 계산된 a, b, c, d 계수를 사용하여 위치 보간
            const FVector2D result = SegmentData.a * tCubed + SegmentData.b * tSquared + SegmentData.c * t + SegmentData.d;
            return result;
        }

        bool FindIntersectionPoint2D(const FVector2D& A, const FVector2D& Dir1, const FVector2D& B, const FVector2D& Dir2, FVector2D& OutIntersection)
        {
            // 두 벡터의 외적을 이용한 분모 계산
            float Denominator = (Dir1.X * Dir2.Y - Dir1.Y * Dir2.X);

            // 평행 또는 매우 가까운 경우 교차 없음
            if (FMath::Abs(Denominator) < KINDA_SMALL_NUMBER)
            {
                return false;
            }

            const FVector2D AB = B - A;
            float t = (AB.X * Dir2.Y - AB.Y * Dir2.X) / Denominator;
            OutIntersection = A + t * Dir1;

            return true;
        }

        TArray<FNavPathPoint> PointsToNavPathPoints(const TArray<FVector>& PathPoints)
        {
            TArray<FNavPathPoint> NavPathPoints;
            for (const FVector& Point : PathPoints)
            {
                NavPathPoints.Add(FNavPathPoint(Point));
            }
            return NavPathPoints;
        }

        TArray<FVector> NavPathPointsToPoints(const TArray<FNavPathPoint>& NavPathPoints)
        {
            TArray<FVector> PathPoints;
            for (const FNavPathPoint& NavPoint : NavPathPoints)
            {
                PathPoints.Add(NavPoint.Location);
            }
            return PathPoints;
        }

    }
}
