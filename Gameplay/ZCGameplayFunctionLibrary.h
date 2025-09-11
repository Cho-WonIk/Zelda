// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "ZCGameplayFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EElementRel : uint8
{
	Weak	UMETA(DisplayName = "약점"),
	Neutral	UMETA(DisplayName = "중립"),
	Strong	UMETA(DisplayName = "강점"),
};

USTRUCT(BlueprintType)
struct FElementInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소"))
	FGameplayTag ElementTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 지속시간"))
	float Duration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "공격자 원소의 확산 카운트"))
	int32 SpreadCount = -1;

	FElementInfo() {}

	FElementInfo(FGameplayTag InElementTag) : ElementTag(InElementTag) {}

	FElementInfo(FGameplayTag InElementTag, float InDuration, int32 InSpreadCount)
	{
		ElementTag = InElementTag;
		Duration = InDuration;
		SpreadCount = InSpreadCount;
	}

	[[nodiscard]] const bool IsEmpty() const { return ElementTag == FGameplayTag::EmptyTag && Duration == -1.0f && SpreadCount == -1; }
	[[nodiscard]] const bool IsNewElement() const { return Duration == -1.0f && SpreadCount == -1; }
};

UCLASS()
class ZELDA_API UZCGameplayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** 지정된 액터에게 일반적인 피해를 가합니다.
	* @param ElementInfo			  적용할 원소 정보
	* @param DamagedActor             피해를 받을 액터.
	* @param BaseDamage               적용할 기본 피해량.
	* @param EventInstigator          이 피해를 유발한 컨트롤러(예: 무기를 발사한 플레이어).
	* @param DamageCauser             실제로 피해를 유발한 액터(예: 폭발한 수류탄).
	* @param DamageTypeClass          가해진 피해를 설명하는 데미지 타입 클래스.
	* @return 액터에게 실제로 적용된 피해량.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ZCGameplay|Damage")
	static float ApplyDamage(FElementInfo& ElementInfo, AActor* DamagedActor, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UZCDamageType> DamageTypeClass);


	/** 지정된 임팩트로 특정 액터에게 피해를 가합니다.
	* @param ElementInfo			  적용할 원소 정보
	* @param DamagedActor             피해를 받을 액터.
	* @param BaseDamage               적용할 기본 피해량.
	* @param HitFromDirection         공격이 날아온 방향.
	* @param HitInfo                  충돌 혹은 트레이스 결과(피격 정보를 담고 있음).
	* @param EventInstigator          이 피해를 유발한 컨트롤러(예: 무기를 발사한 플레이어).
	* @param DamageCauser             실제로 피해를 유발한 액터(예: 폭발한 수류탄).
	* @param DamageTypeClass          가해진 피해를 설명하는 데미지 타입 클래스.
	* @return 액터에게 실제로 적용된 피해량.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ZCGameplay|Damage")
	static float ApplyPointDamage(FElementInfo& ElementInfo, AActor* DamagedActor, float BaseDamage, FVector const& HitFromDirection, FHitResult const& HitInfo, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UZCDamageType> DamageTypeClass);

	/** 반경 내에서 로컬 권한이 있는 액터들에게 피해를 가합니다.
	*    Visibility 채널설정이 블록인 컴포넌트만 타격합니다.
	* @param ElementInfo			  적용할 원소 정보
	* @param WoldContextObject		  GetWorld()를 할 수 있는 객체(보통 this)
	* @param BaseDamage               적용할 기본 피해량, 즉 진원지에서의 피해.
	* @param Origin                   피해 영역의 진원지.
	* @param DamageRadius             Origin으로부터의 피해 반경.
	* @param DamageTypeClass          가해진 피해를 설명하는 데미지 타입 클래스.
	* @param IgnoreActors             피해 계산에서 무시할 액터 목록.
	* @param DamageCauser             실제로 피해를 유발한 액터(예: 폭발한 수류탄).
	*                                 이 액터는 피해를 받지 않으며 피해를 방해하지도 않습니다.
	* @param InstigatedByController   이 피해를 유발한 컨트롤러(예: 수류탄을 던진 플레이어).
	* @param bFullDamage              true이면 Origin으로부터의 거리와 관계없이
	*                                 피해가 감소하지 않습니다.
	* @param DamagePreventionChannel  Origin과 대상 사이에 이 채널을 차단하는 무언가가 있으면
	*                                 피해가 적용되지 않습니다.
	*
	* @return 하나 이상의 액터에게 피해가 적용되면 true를 반환합니다.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ZCGameplay|Damage", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreActors"))
	static bool ApplyRadialDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FVector& Origin, float DamageRadius, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = nullptr, AController* InstigatedByController = nullptr, bool bDoFullDamage = false, ECollisionChannel DamagePreventionChannel = ECC_Visibility);

	static bool ApplyRadialDamageWithFalloff(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = nullptr, AController* InstigatedByController = nullptr, ECollisionChannel DamagePreventionChannel = ECC_Visibility);

	/** 데미지를 계산합니다.
	* @param BaseDamage			기본 피해량.
	* @param ElementRelation	원소 속성의 약점, 중립, 강점 관계
	* @parm  bIsCriticalBone	약점 부위 피격 여부
	* @parm	 ArmorState			피격자의 방어력
	* 
	* @return FinalDamage		계산된 최종 피해량
	*/
	UFUNCTION(BlueprintCallable, Category = "ZCGameplay|Damage")
	static float CalculateDamage(float BaseDamage, EElementRel ElementResult = EElementRel::Neutral, bool IsCriticalBone = false, float ArmorState = 0.0f);
};
