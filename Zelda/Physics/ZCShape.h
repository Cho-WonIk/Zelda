// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CollisionShape.h"
#include "ZCShape.generated.h"

UENUM(BlueprintType)
enum class EZCShapeType : uint8
{
	Line	UMETA(DisplayName = "Line"),
	Box		UMETA(DisplayName = "Box"),
	Sphere	UMETA(DisplayName = "Sphere"),
	Capsule UMETA(DisplayName = "Capsule")
};

USTRUCT(BlueprintType)
struct FZCShape
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "종류"))
	EZCShapeType Type;

	// Line, Shpere : X값만, Capsule : X축 반지름, Y축 Half-height, Box : X, Y, Z값
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "도형 정보"))
	FVector Info;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "도형 각도"))
	FRotator Rotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "도형 시작 위치"))
	FVector Start;

	FCollisionShape ToCollisionShape() const
	{
		switch (Type)
		{
		case EZCShapeType::Box:
		{
			// Half extents는 최소값 이하로 내려가지 않도록 보정
			const FVector3f Min(FCollisionShape::MinBoxExtent());
			const FVector3f Half = FVector3f(Info).ComponentMax(Min);
			return FCollisionShape::MakeBox(Half);
		}

		case EZCShapeType::Sphere:
		{
			const float R = FMath::Max(static_cast<float>(Info.X), FCollisionShape::MinSphereRadius());
			return FCollisionShape::MakeSphere(R);
		}

		case EZCShapeType::Capsule:
		{
			const float Radius = FMath::Max(static_cast<float>(Info.X), FCollisionShape::MinCapsuleRadius());
			const float HalfHeight = FMath::Max(static_cast<float>(Info.Y), FCollisionShape::MinCapsuleAxisHalfHeight());

			return FCollisionShape::MakeCapsule(Radius, HalfHeight);
		}

		case EZCShapeType::Line:
		default:
			return FCollisionShape(); // 라인 트레이스
		}
	}

	void BuildWorldQuery(const FTransform& OwnerTransform, FVector& OutOrigin, FQuat& OutRotation, FCollisionShape& OutShape) const
	{
		OutOrigin = BuildWorldPosition(OwnerTransform);
		OutRotation = BuildWorldRotation(OwnerTransform);
		OutShape = ToCollisionShape();
	}

	FQuat BuildWorldRotation(const FTransform& OwnerTransform) const
	{
		const FQuat Base = OwnerTransform.GetRotation() * Rotation.Quaternion();

		if (Type == EZCShapeType::Capsule)
		{
			// 디버그만 롤 +90
			return Base * FQuat(FRotator(0.f, 0.f, 90.f));
		}
		return Base;
	}

	/** 디버그 드로잉/물리 쿼리 공통 위치 */
	FVector BuildWorldPosition(const FTransform& OwnerTransform) const
	{
		return OwnerTransform.TransformPosition(Rotation.RotateVector(Start));
	}
};

#if !UE_BUILD_SHIPPING
namespace Zelda
{
	/**
	* 인게임 월드에서 FZCShape를 디버그 드로잉합니다.
	*
	* @param World             그릴 대상 월드
	* @param Shape             그릴 Shape 데이터
	* @param OwnerTransform    Shape의 기준이 되는 부모 트랜스폼
	* @param Color             드로잉 색상
	* @param Thickness         드로잉 두께
	* @param Duration          드로잉 유지 시간 (0이면 1프레임)
	* @param DepthPriority     깊이 우선순위
	* @param bPersistentLines  true이면 Duration과 관계없이 계속 유지됨
	* @param bShowLineEndpoints Line 타입일 경우 끝점에 구를 표시할지 여부
	* @param LineEndpointSphereRadius 끝점 구의 반지름
	*/
	static void DrawDebugShape(UWorld* World, const FZCShape& Shape, const FTransform& OwnerTransform, const FColor Color = FColor::Cyan, float Thickness = 1.f, float Duration = 0.f, uint8 DepthPriority = 0, bool bPersistentLines = false, bool bShowLineEndpoints = false, float LineEndpointSphereRadius = 5.f)
	{
		if (!World) return;

		// const FQuat WorldRot = OwnerTransform.GetRotation(); 캡슐일때 올바른 방향, Roll방향으로 90도 회전시켜주어야한다.

		// const FQuat WorldRot = OwnerTransform.GetRotation()* Shape.Rotation.Quaternion(); 박스일때 올바른 방향
		// const FQuat WorldRot = OwnerTransform.GetRotation()* Shape.Rotation.Quaternion(); 스페어 일때 올바른 방향
		// const FQuat WorldRot = OwnerTransform.GetRotation()* Shape.Rotation.Quaternion(); 라인 일때 올바른 방향

		const FVector Pos = Shape.BuildWorldPosition(OwnerTransform);
		const FQuat   Rot = Shape.BuildWorldRotation(OwnerTransform);

		switch (Shape.Type)
		{
		case EZCShapeType::Box:
		{
			DrawDebugBox(World, Pos, Shape.Info, Rot, Color, bPersistentLines, Duration, DepthPriority, Thickness);
			break;
		}
		case EZCShapeType::Sphere:
		{
			DrawDebugSphere(World, Pos, Shape.Info.X, 32, Color, bPersistentLines, Duration, DepthPriority, Thickness);
			break;
		}
		case EZCShapeType::Capsule:
		{
			const float HalfHeight = Shape.Info.Y;
			const float Radius = Shape.Info.X;
			DrawDebugCapsule(World, Pos, HalfHeight, Radius, Rot, Color, bPersistentLines, Duration, DepthPriority, Thickness);
			break;
		}
		case EZCShapeType::Line:
		{
			const FVector Dir = Rot.GetAxisX();
			const FVector End = Pos + Dir * Shape.Info.X;
			DrawDebugLine(World, Pos, End, Color, bPersistentLines, Duration, DepthPriority, Thickness);

			if (bShowLineEndpoints)
			{
				DrawDebugSphere(World, Pos, LineEndpointSphereRadius, 8, Color, bPersistentLines, Duration, DepthPriority, Thickness);
				DrawDebugSphere(World, End, LineEndpointSphereRadius, 8, Color, bPersistentLines, Duration, DepthPriority, Thickness);
			}
			break;
		}
		}
	}
}
#endif // !UE_BUILD_SHIPPING

