// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Gameplay/GameplayTag/ZCGameplayTag.h"
#include "GameData/Struct/ZCChemistryStruct.h"
#include "ZCGameplayFunctionLibrary.generated.h"

struct FZCShape;

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
	*   Default설정 : Visibility 채널설정이 블록인 컴포넌트만 타격합니다.
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

	/** 반경 내에서 Shape 모양으로 액터들에게 피해를 가합니다.
	*   Default 설정 : Visibility 채널설정이 블록인 컴포넌트만 타격합니다.
	*
	* @param ElementInfo              적용할 원소 정보.
	* @param WorldContextObject       GetWorld()를 할 수 있는 객체(보통 this).
	* @param BaseDamage               적용할 기본 피해량.
	* @param Origin                   피해 영역의 중심 위치.
	* @param DamageShape              피해를 가할 범위의 모양(예: 구, 캡슐, 박스 등).
	* @param DamageTypeClass          가해진 피해를 설명하는 데미지 타입 클래스.
	* @param IgnoreActors             피해 계산에서 무시할 액터 목록.
	* @param DamageCauser             실제로 피해를 유발한 액터(예: 폭발한 오브젝트).
	*                                 이 액터는 피해를 받지 않으며 피해를 방해하지 않습니다.
	* @param InstigatedByController   이 피해를 유발한 컨트롤러(예: 공격을 실행한 플레이어).
	* @param DamagePreventionChannel  Origin과 대상 사이에 이 채널을 차단하는 무언가가 있으면
	*                                 피해가 적용되지 않습니다.
	*
	* @return 하나 이상의 액터에게 피해가 적용되면 true를 반환합니다.
	*/
	static bool ApplyShapeDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FTransform& OwnerTransform, const FZCShape& ZCDamageShape, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = nullptr, AController* InstigatedByController = nullptr, ECollisionChannel DamagePreventionChannel = ECC_Visibility);


	/** 가장 접촉한 액터에게 피해를 가합니다.
	* Default 설정 : Visibility 채널 설정이 컴포넌트만 타격합니다.
	*
	* @param ElementInfo				적용한 원소 정보
	* @param WorldContextObject			GetWorld()를 할 수 있는 객체(보통 this)
	* @param BaseDamage					적용할 기본 피해량
	* @param Start						시작 벡터
	* @param Direction					시작 벡터에서 트레이싱할 방향벡터
	* @param DamageTypeClass			가해진 피해를 설명하는 데미지 타입 클래스.
	* @param IgnoreActors				피해 계산에서 무시할 액터 목록.
	* @param DamageCauser				실제로 피해를 유발한 액터(예: 폭발한 오브젝트).
	*									이 액터는 피해를 받지 않으며 피해를 방해하지 않습니다.
	* @param InstigatedByController		이 피해를 유발한 컨트롤러(예: 공격을 실행한 플레이어).
	* @param DamagePreventionChannel	Origin과 대상 사이에 이 채널을 차단하는 무언가가 있으면
	*									피해가 적용되지 않습니다.
	*
	* @return 액터에게 피해가 적용되면 true를 반환합니다.
	*/
	static bool ApplyTouchDamage(FElementInfo& ElementInfo, const UObject* WorldContextObject, float BaseDamage, const FVector& Start, const FVector& Direction, TSubclassOf<UZCDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = nullptr, AController* InstigatedByController = nullptr, ECollisionChannel DamagePreventionChannel = ECC_Visibility);

};
