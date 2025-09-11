// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Struct/Enum/ZCMonster.h"

class AZCWeaponActor;
class AZCShieldActor;

#include "ZCAIInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UZCAIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZELDA_API IZCAIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zelda|AI")
	void AttackByAI(const FVector& TargetLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zelda|AI")
	void DefenseByAI(const FVector& TargetLocation = FVector::ZeroVector);

	virtual void EquipWeaponByAI(AZCWeaponActor* Weapon) = 0;
	virtual void EquipShieldByAI(AZCShieldActor* Shield) = 0;

	virtual EMonsterRole GetMonsterRole() const = 0;
	virtual EMonsterType GetMonsterType() const = 0;
};
