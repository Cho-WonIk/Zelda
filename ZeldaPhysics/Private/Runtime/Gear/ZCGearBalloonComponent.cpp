// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearBalloonComponent.h"

#include "Core/Module/ZCSimGearBalloonModule.h"

ZCGear::IZCSimGearBaseModule* UZCGearBalloonComponent::CreateNewGearModule() const
{
	ZCGear::FZCBalloonSettings Settings;
	Settings.MaxForce = MaxForce;
	Settings.MaxSpeed = MaxSpeed;

	ZCGear::IZCSimGearBaseModule* NewModule = new ZCGear::FZCSimGearBalloonModule(Settings);

	return NewModule;
}
