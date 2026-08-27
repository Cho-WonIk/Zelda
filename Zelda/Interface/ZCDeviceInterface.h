// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZCDeviceInterface.generated.h"

class AZCActor;


// 울트라 핸드 인터페이스
UINTERFACE(MinimalAPI)
class UZCUltrahandDeviceInterface : public UInterface
{
	GENERATED_BODY()
};

// 월드 오브젝트에서 플레이어 스킬과 상호작용하기 위해 구현
class ZELDA_API IZCUltrahandDeviceInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void AddAssembleCandidate(AZCActor* OwnerActor, AZCActor* AddActor) = 0;
	virtual void RemoveAssembleCandidate(AZCActor* OwnerActor, AZCActor* RemoveActor) = 0;
};

