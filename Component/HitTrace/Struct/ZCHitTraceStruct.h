#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ZCHitTraceStruct.generated.h"

namespace ZCHitTraceSockets
{
	static const FName Top(TEXT("Top"));
	static const FName Bottom(TEXT("Bottom"));
}

UENUM(BlueprintType)
enum class EHitTraceType : uint8
{
    Channel     UMETA(DisplayName = "채널"),
    ObjectType  UMETA(DisplayName = "오브젝트"),
    ProfileName UMETA(DisplayName = "프로파일")
};

USTRUCT(BlueprintType)
struct FZCHitTraceStruct
{
	GENERATED_BODY()

public:
	// 기본 생성자
	FZCHitTraceStruct()
		: Mesh(nullptr)
		, Instigator(nullptr)
		, bAllowMultipleHit(false)
		, HitInterval(0.f)
		, TraceComplex(false)
		, bIgnoreSelf(true)
		, Radius(10.f)
		, BeginSocket(NAME_None)
		, EndSocket(NAME_None)
		, ProfileName(NAME_None)
		, TraceType(EHitTraceType::Channel)
		, TraceChannel(TraceTypeQuery1)
		, DrawDebugType(EDrawDebugTrace::None)
	{
	}

	static const FZCHitTraceStruct ObjectDefault;
	static const FZCHitTraceStruct ChannelDefault;
	static const FZCHitTraceStruct ProfileDefault;

public:
	/** 트레이스 기준이 되는 메시 또는 소켓이 포함된 컴포넌트 */
	UPROPERTY()
	USceneComponent* Mesh;

	/** 트레이스를 수행한 주체(예: 무기 소유자 또는 캐릭터) */
	UPROPERTY()
	AActor* Instigator;

	/** 동일 액터에 대해 다단히트를 허용할지 여부 */
	UPROPERTY()
	bool bAllowMultipleHit;

	/** 동일 액터에 대해 다단히트 발생 시 허용 간격 (초 단위) */
	UPROPERTY()
	float HitInterval;

	/** 트레이스를 복잡한 충돌 형태(Complex Collision)로 수행할지 여부 */
	UPROPERTY()
	bool TraceComplex;

	/** Instigator 자신을 트레이스 충돌 대상에서 제외할지 여부 */
	UPROPERTY()
	bool bIgnoreSelf;

	/** 트레이스 영역의 반지름 (Sphere 또는 Capsule 사용 시 적용) */
	UPROPERTY()
	float Radius;

	/** 트레이스 시작 지점을 나타내는 소켓 이름 */
	UPROPERTY()
	FName BeginSocket;

	/** 트레이스 종료 지점을 나타내는 소켓 이름 */
	UPROPERTY()
	FName EndSocket;

	/** TraceType이 ProfileName일 때 사용되는 Collision Profile 이름 */
	UPROPERTY()
	FName ProfileName;

	/** 트레이스 타입 설정 (Channel, ObjectType, ProfileName 중 선택) */
	UPROPERTY()
	EHitTraceType TraceType;

	/** TraceType이 Channel일 때 사용되는 채널 값 */
	UPROPERTY()
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	/** 트레이스 결과를 디버그로 시각화할 때의 출력 타입 */
	UPROPERTY()
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;

	/** 트레이스 시 무시할 액터 리스트 */
	UPROPERTY()
	TArray<AActor*> IgnoreActors;

	/** TraceType이 ObjectType일 때 사용할 오브젝트 타입 목록 */
	UPROPERTY()
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** 이미 피격 처리된 액터들을 저장하여 중복 피격을 방지하기 위한 집합 */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> TracedActors;
};
