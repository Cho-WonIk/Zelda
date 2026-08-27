// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ZCGearUtility.h"
#include "Chaos/DebugDrawQueue.h"

namespace ZCGear
{
	void FZCDebugDraw::Line(const FVector& Start, const FVector& End, const FColor& Color, float Thickness, float LifeTime)
	{
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugLine(Start, End, Color, false, LifeTime, 0, Thickness);
	}

	void FZCDebugDraw::Arrow(const FVector& Start, const FVector& End, float ArrowSize, const FColor& Color, float Thickness, float LifeTime)
	{
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Start, End, ArrowSize, Color, false, LifeTime, 0, Thickness);
	}

	void FZCDebugDraw::Sphere(const FVector& Center, float Radius, const FColor& Color, float LifeTime)
	{
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugSphere(Center, Radius, 12, Color, false, LifeTime, 0, 1.0f);
	}

	void FZCDebugDraw::Box(const FVector& Center, const FVector& Extents, const FQuat& Rotation, const FColor& Color, float LifeTime)
	{
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(Center, Extents, Rotation, Color, false, LifeTime, 0, 1.0f);
	}

	void FZCDebugDraw::CoordinateSystem(const FVector& Location, const FRotator& Rotation, float Scale, float LifeTime)
	{
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugCoordinateSystem(Location, Rotation, Scale, false, LifeTime, 0, 1.0f);
	}
}