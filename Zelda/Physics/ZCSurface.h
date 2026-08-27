// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "ZCSurface.generated.h"


// 접촉했던 위치 정보, 물체와 접촉이 지속되는 중인지 확인하는 용도
struct FZCSurfaceInfo
{
	FVector LocalLocation;
	FVector LocalDirection;

	FZCSurfaceInfo() = default;

	FZCSurfaceInfo(const FVector& InLocation, const FVector& InDirection) : LocalLocation(InLocation), LocalDirection(InDirection)
	{
	}
};