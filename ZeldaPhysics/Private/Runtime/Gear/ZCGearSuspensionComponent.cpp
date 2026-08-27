// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearSuspensionComponent.h"

#include "Core/Module/ZCSimGearSuspensionModule.h"

ZCGear::IZCSimGearBaseModule* UZCGearSuspensionComponent::CreateNewGearModule() const
{
	ZCGear::FZCSuspensionSettings Settings;

	Settings.MaxYawAngle = MaxYawAngle;
	Settings.SteerSpeed = SteerSpeed;

	AActor* Owner = GetOwner();

	if (Owner && Owner->GetRootComponent())
	{
		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
		{
			FTransform RootTrans = RootPrim->GetComponentTransform();
			FTransform SuspensionTrans = GetComponentTransform();
			FTransform RelativeTrans = SuspensionTrans.GetRelativeTransform(RootTrans);

			Settings.InitialRelativeRotation = RelativeTrans.GetRotation();
		}
	}

	ZCGear::IZCSimGearBaseModule* NewModule = new ZCGear::FZCSimGearSuspensionModule(Settings);

	return NewModule;
}
