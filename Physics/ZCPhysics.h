
#pragma once
#include "CoreMinimal.h"
#include "ZCPhysics.generated.h"

UENUM(BlueprintType)
enum class EZCGravityType : uint8
{
	Default = 0,		// 기본 중력
	LowGravity = 1,		// 저 중력
	NoGravity = 2		// 무중력
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EZCAcceleration : uint8
{
	None = 0											UMETA(DisplayName = "None"),

	Upward = 1 << 0										UMETA(DisplayName = "Upward"),
	Downward = 1 << 1									UMETA(DisplayName = "Downward"),
	Forward = 1 << 2									UMETA(DisplayName = "Forward"),
	Backward = 1 << 3									UMETA(DisplayName = "Backward"),
	Leftward = 1 << 4									UMETA(DisplayName = "Leftward"),
	Rightward = 1 << 5									UMETA(DisplayName = "Rightward"),

	// ───────────────────────
	// 2방향 조합
	// ───────────────────────
	UpForward = Upward | Forward						UMETA(DisplayName = "Up + Forward"),
	UpBackward = Upward | Backward						UMETA(DisplayName = "Up + Backward"),
	UpLeft = Upward | Leftward							UMETA(DisplayName = "Up + Left"),
	UpRight = Upward | Rightward						UMETA(DisplayName = "Up + Right"),

	DownForward = Downward | Forward					UMETA(DisplayName = "Down + Forward"),
	DownBackward = Downward | Backward					UMETA(DisplayName = "Down + Backward"),
	DownLeft = Downward | Leftward						UMETA(DisplayName = "Down + Left"),
	DownRight = Downward | Rightward					UMETA(DisplayName = "Down + Right"),

	ForwardLeft = Forward | Leftward					UMETA(DisplayName = "Forward + Left"),
	ForwardRight = Forward | Rightward					UMETA(DisplayName = "Forward + Right"),
	BackwardLeft = Backward | Leftward					UMETA(DisplayName = "Backward + Left"),
	BackwardRight = Backward | Rightward				UMETA(DisplayName = "Backward + Right"),

	// ───────────────────────
	// 3방향 조합
	// ───────────────────────
	UpForwardLeft = Upward | Forward | Leftward			UMETA(DisplayName = "Up + Forward + Left"),
	UpForwardRight = Upward | Forward | Rightward		UMETA(DisplayName = "Up + Forward + Right"),
	UpBackwardLeft = Upward | Backward | Leftward		UMETA(DisplayName = "Up + Backward + Left"),
	UpBackwardRight = Upward | Backward | Rightward		UMETA(DisplayName = "Up + Backward + Right"),

	DownForwardLeft = Downward | Forward | Leftward		UMETA(DisplayName = "Down + Forward + Left"),
	DownForwardRight = Downward | Forward | Rightward	UMETA(DisplayName = "Down + Forward + Right"),
	DownBackwardLeft = Downward | Backward | Leftward	UMETA(DisplayName = "Down + Backward + Left"),
	DownBackwardRight = Downward | Backward | Rightward	UMETA(DisplayName = "Down + Backward + Right"),
};
ENUM_CLASS_FLAGS(EZCAcceleration);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EZCPhysicsFlags : uint8
{
	// 어떠한 물리적 상호작용 없음( Ex : 말뚝 고정 상태)
	None = 0,
	// 언리얼 기본
	Default = 1 << 0,
	// 중력 설정
	Gravity = 1 << 1,
	// 힘이 가해짐
	Acceleration = 1 << 2,
	// 자체적으로 균형을 잡음
	SelfBalancing = 1 << 3,
	// 축 중심 회전
	Rotate = 1 << 4,
	// 추적 기능
	Tracking = 1 << 5

	// 프리셋
};
ENUM_CLASS_FLAGS(EZCPhysicsFlags);

USTRUCT(BlueprintType)
struct FPhysicsSetting
{
	GENERATED_BODY()

public:
	// 기본 물리 시뮬레이션에서 변경사항이 필요할 경우 토글
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = EZCPhysicsFlags))
	uint8 PhysicsFlags = static_cast<uint8>(EZCPhysicsFlags::Default);

	// Acceleration이 true인 경우 가해지는 힘의 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = EZCAcceleration, EditCondition = "PhysicsFlags & EZCPhysicsFlags::Acceleration", EditConditionHides))
	uint8  Acceleration;

	// Acceleration이 true인 경우 가해지는 힘의 크기 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = EZCAcceleration, EditCondition = "PhysicsFlags & EZCPhysicsFlags::Acceleration", EditConditionHides))
	float  AccelerationScale;

	// Gravity가 true인경우 중력 옵션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "PhysicsFlags & EZCPhysicsFlags::Gravity", EditConditionHides))
	EZCGravityType Gravity;

	// EZCGravityType이 LowGravity인경우 중력 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Gravity == EZCGravityType::LowGravity", EditConditionHides))
	float GravityScale = 1.0f;

};
