// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameData/ZCItemTable.h"
#include "ZCItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZCItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZELDA_API IZCItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 아이템의 ID를 반환
	virtual int32 GetID() const = 0;
	// 아이템의 타입을 반환
	virtual EItemType GetType() const = 0;
	// 아이템의 정보를 반환
	virtual const FZCItemTable* GetInfo() const = 0;
	// 아이템 습득
	virtual TUniquePtr<FZCItemTable> OnItemPickUp() = 0;
};
