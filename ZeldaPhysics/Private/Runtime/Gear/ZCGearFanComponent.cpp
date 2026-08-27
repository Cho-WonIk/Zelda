// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearFanComponent.h"

#include "Core/Module/ZCSimGearFanModule.h"

ZCGear::IZCSimGearBaseModule* UZCGearFanComponent::CreateNewGearModule() const
{
	ZCGear::FZCFanSettings Settings;
	Settings.DefaultForce = WindStrength;
	Settings.MaxSpeed = MaxWindSpeed;
	Settings.PGain = PGain;
	Settings.Axis = ZCGear::EZCSimGearAxis::Y;

	ZCGear::IZCSimGearBaseModule* NewModule = new ZCGear::FZCSimGearFanModule(Settings);

	return NewModule;
}
