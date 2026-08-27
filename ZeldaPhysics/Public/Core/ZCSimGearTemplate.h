// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 기어 시뮬레이션 시스템의 설정값을 가져올 수 있는 API
template<typename T>
class TZCGearSystemTemplate
{
public:
	
public:
	TZCGearSystemTemplate() : SetupPtr(nullptr)
	{}

	TZCGearSystemTemplate(const T* InSetup) : SetupPtr(InSetup)
	{
		check(SetupPtr != nullptr);
	}

	FORCEINLINE T& AccessSetup()
	{
		check(SetupPtr != nullptr);
		return (T&)(*SetupPtr);
	}

	FORCEINLINE const T& Setup() const
	{
		check(SetupPtr != nullptr);
		return (*SetupPtr);
	}

	const T* SetupPtr;
};
