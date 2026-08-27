// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/Gear/ZCGearWheelComponent.h"
#include "Core/Module/ZCSimGearWheelModule.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

ZCGear::IZCSimGearBaseModule* UZCGearWheelComponent::CreateNewGearModule() const
{
	ZCGear::FZCWheelSettings Settings;
	Settings.DefaultSpeed = 10.0f;
	Settings.MaxYawAngle = 22.5f;

    // 1. Source(루트) 프록시 찾기
    AActor* Owner = GetOwner();
    IPhysicsProxyBase* SourceProxy = nullptr;

    if (Owner && Owner->GetRootComponent())
    {
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
        {
            if (FBodyInstance* BI = RootPrim->GetBodyInstance())
            {
                SourceProxy = BI->GetPhysicsActor();
            }

            // 2. 초기 상대 회전값 계산 (Root 기준 Wheel의 로컬 회전)
            // 차체가 기울어져 있어도 바퀴가 올바른 초기 각도를 알도록 저장합니다.
            FTransform RootTrans = RootPrim->GetComponentTransform();
            FTransform WheelTrans = GetComponentTransform();

            FTransform RelativeTrans = WheelTrans.GetRelativeTransform(RootTrans);
            Settings.InitialRelativeLocation = RelativeTrans.GetLocation();
            Settings.InitialRelativeRotation = WheelTrans.GetRelativeTransform(RootTrans).GetRotation();
        }
    }

    Settings.SourcePhysicsProxy = SourceProxy;

	ZCGear::IZCSimGearBaseModule* NewModule = new ZCGear::FZCSimGearWheelModule(Settings);

	return NewModule;
}
