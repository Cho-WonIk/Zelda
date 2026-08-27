// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/ZCGearType.h"
#include "ZCGearBaseComponent.generated.h"

class IPhysicsProxyBase;

namespace ZCGear
{
	class IZCSimGearBaseModule;
}

/** 물리 핸들을 가져올 대상을 정의하는 열거형 */
UENUM(BlueprintType)
enum class EZCGearTargetMode : uint8
{
	Self                UMETA(DisplayName = "현재 컴포넌트"),
	Root                UMETA(DisplayName = "루트 컴포넌트"),
	SpecificComponent   UMETA(DisplayName = "타겟컴포넌트")
};

UCLASS()
class ZELDAPHYSICS_API UZCGearBaseComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UZCGearBaseComponent(const FObjectInitializer& ObjectInitializer);

	virtual EZCGearType GetGearType() const { return EZCGearType::Undefined; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const { return nullptr; }

	IPhysicsProxyBase* GetCurrentParticleHandle();

protected:
	void FindCurrentParticleHandle();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear")
	EZCGearTargetMode TargetMode = EZCGearTargetMode::Self;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZeldaPhysics|Gear", meta = (EditCondition = "TargetMode == EZCGearTargetMode::SpecificComponent"))
	FName TargetComponentName;

	IPhysicsProxyBase* CurrentProxy = nullptr;

};
