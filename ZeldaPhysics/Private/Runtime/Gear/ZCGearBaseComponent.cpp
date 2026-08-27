// Fill out your copyright notice in the Description page of Project Settings.

#include "Runtime/Gear/ZCGearBaseComponent.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"


UZCGearBaseComponent::UZCGearBaseComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

IPhysicsProxyBase* UZCGearBaseComponent::GetCurrentParticleHandle()
{
	if (CurrentProxy) return CurrentProxy;

	FindCurrentParticleHandle();

	return CurrentProxy;
}

void UZCGearBaseComponent::FindCurrentParticleHandle()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* TargetComp = nullptr;

    switch (TargetMode)
    {
    case EZCGearTargetMode::Self:
        TargetComp = this;
        break;

    case EZCGearTargetMode::Root:
        TargetComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
        break;

    case EZCGearTargetMode::SpecificComponent:
    {
		ensure(TargetComponentName != NAME_None);
        TArray<UPrimitiveComponent*> PrimComps;
        Owner->GetComponents<UPrimitiveComponent>(PrimComps);

        for (UPrimitiveComponent* Comp : PrimComps)
        {
            if (Comp && Comp->GetFName() == TargetComponentName)
            {
                TargetComp = Comp;
                break;
            }
        }
    }
    break;
    }

    if (TargetComp)
    {
        if (FBodyInstance* BI = TargetComp->GetBodyInstance())
        {
            CurrentProxy = BI->GetPhysicsActor();
        }
    }
}
