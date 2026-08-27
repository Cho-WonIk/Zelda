// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Gear/ZCGearBaseComponent.h"
#include "ZCGearInputComponent.generated.h"

class FZCSimGearManager;
class UZCGearMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDAPHYSICS_API UZCGearInputComponent : public UZCGearBaseComponent
{
	GENERATED_BODY()

public:
	UZCGearInputComponent(const FObjectInitializer& ObjectInitializer);

public:
	virtual EZCGearType GetGearType() const override { return EZCGearType::Input; }
	virtual ZCGear::IZCSimGearBaseModule* CreateNewGearModule() const override;

	void StartRide(const TArray<UZCGearMovementComponent*>& ConnectedGearMovements);
	void EndRide(const TArray<UZCGearMovementComponent*>& ConnectedGearMovements);

	void ApplyPlayerInput(const FVector2D& RawInput);

	void SetCenterOfMassOffset(const FVector& InCenterOfMassOffset, const float InMass);
	FVector GetCenterOfMassOffset() const { return CenterOfMassOffset; }

protected:
	void FindGearManager();

	FVector CalculatePlayerViewLocalVector() const;

protected:
	// 기어 시뮬레이션 매니저
	FZCSimGearManager* GearManager = nullptr;

	// 인풋 컴포넌트에서 연결된 전체 액터들의 무게 중심 오프셋
	FVector CenterOfMassOffset = FVector::ZeroVector;

	// 연결된 컴포넌트들의 총 무게
	float TotalMass = 0.0f;
};
