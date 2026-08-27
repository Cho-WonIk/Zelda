
#pragma once
#include "CoreMinimal.h"
#include "GameData/Enum/ZCDirection.h"
#include "ZCPhysics.generated.h"

USTRUCT(BlueprintType)
struct FZCMovementData
{
	GENERATED_BODY()

public:
	// 가해질 힘의 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = EZCDirection))
	uint8 Direction = 0;

	// 목표 속력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Speed = 0.0f;

	// 목표 가속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Accele = 0.0f;

	// 정상적으로 목표 속력과 가속도에 도달할 수 있는 최대 중량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxMass = 0.0f;
};

USTRUCT(BlueprintType)
struct FZCGearParam
{
	GENERATED_BODY()

public:
	// 월드의 Z축을 기준으로 균형을 잡을 지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnableBalancing;

	// 균형을 잡는 다면 균형을 잡을 복원력 최대 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float BalancingForce;

	// 실제 물리엔진의 중력을 적용할 지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnablePhysicsGravity = true;

	// 가짜 중력을 적용할 지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bEnableGravity = false;

	// 가짜 중력 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float GravityScale = 1.0f;

public:
	// 물체 기준 (Local) 이동 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Local")
	FZCMovementData Local;

	// 월드 기준 (World) 이동 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|World")
	FZCMovementData World;
};
