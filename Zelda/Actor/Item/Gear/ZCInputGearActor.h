// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Item/Gear/ZCGearActor.h"
#include "ZCInputGearActor.generated.h"

class UZCGearInputComponent;
class UArrowComponent;

UCLASS()
class ZELDA_API AZCInputGearActor : public AZCGearActor
{
	GENERATED_BODY()
	
public:
	AZCInputGearActor(const FObjectInitializer& ObjectInitializer);

public:
	FName GetRideSocketName() const { return RideSocketName; }

public:
	void RideStart();
	void RideEnd();

	void GetPlayerInput(const FVector2D& RawInput);

protected:
	virtual void OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

protected:
	FVector CalculateCenterOfMassOffset(float& InMass);

#if WITH_EDITOR
	// 에디터에서 소켓 리스트를 생성하여 반환하는 함수
	UFUNCTION()
	TArray<FString> GetSocketNames() const;
#endif // WITH_EDITOR

protected:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZCGearInputComponent> GearInputComponent;

	// 탑승 방향을 안내할 화살표 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gear", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> RideDirectionArrow;

	// 탑승시 캐릭터를 부착할 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gear", meta = (AllowPrivateAccess = "true", DisplayName = "탑승 소켓 이름", GetOptions = "GetSocketNames"))
	FName RideSocketName = NAME_None;
};
