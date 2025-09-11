// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZCCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZCCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZELDA_API IZCCombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 공격 판정이 가능한지 여부
	 * @param bCanHit - 공격 판정 가능 여부
	 * @param bAllowMultipleHit - 다단 히트 여부
	 * @param HitInterval - 다단 히트 간격
	 */
	virtual void OnTraceHit(bool bCanHit, bool bAllowMultipleHit, float HitInterval) = 0;

	virtual void OnParry(bool bCanParry) = 0;
};
