// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gear/ZCGearActor.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCGearActor)

AZCGearActor::AZCGearActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void AZCGearActor::BeginPlay()
{
	Super::BeginPlay();
	InitAllConstraints();
}

void AZCGearActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TermAllConstraints();
	Super::EndPlay(EndPlayReason);
}

int32 AZCGearActor::AddConstraint(UPrimitiveComponent* Component1, FName BoneName1, UPrimitiveComponent* Component2, FName BoneName2, const FConstraintInstance& TemplateInstance)
{
	const int32 Comp1Idx = FindOrAddComponent(Component1);
	const int32 Comp2Idx = FindOrAddComponent(Component2);
	const int32 Bone1Idx = FindOrAddBoneName(BoneName1);
	const int32 Bone2Idx = FindOrAddBoneName(BoneName2);

	const int32 ConstraintIdx = ConstraintInstances.Add(TemplateInstance);

	FIndexedConstraintJoint NewJoint;
	NewJoint.ConstraintIndex = (uint16)ConstraintIdx;
	NewJoint.Component1Index = (uint16)Comp1Idx;
	NewJoint.Component2Index = (uint16)Comp2Idx;
	NewJoint.BoneName1Index = (uint16)Bone1Idx;
	NewJoint.BoneName2Index = (uint16)Bone2Idx;
	NewJoint.bInitialized = false;

	const int32 JointHandle = Joints.Add(NewJoint);

	if (HasActorBegunPlay())
	{
		InitConstraintInternal(JointHandle);
	}

	return JointHandle;
}

void AZCGearActor::RemoveConstraint(int32 JointHandle)
{
	if (!Joints.IsValidIndex(JointHandle)) return;

	TermConstraintInternal(JointHandle);
	Joints.RemoveAt(JointHandle);
}

void AZCGearActor::BreakConstraint(int32 JointHandle, bool bKeepData)
{
	if (!Joints.IsValidIndex(JointHandle)) return;

	FConstraintInstance* Instance = GetConstraintInstance(JointHandle);
	if (Instance)
	{
		Instance->TermConstraint();
	}

	if (!bKeepData)
	{
		RemoveConstraint(JointHandle);
	}
	else
	{
		Joints[JointHandle].bInitialized = false;
	}
}

void AZCGearActor::InitAllConstraints()
{
	for (auto i = 0; i < Joints.Num(); ++i)
	{
		InitConstraintInternal(i);
	}
}

void AZCGearActor::TermAllConstraints()
{
	for (int32 i = 0; i < Joints.Num(); ++i)
	{
		TermConstraintInternal(i);
	}
}

bool AZCGearActor::GetConstraintInfo(int32 JointHandle, UPrimitiveComponent*& OutComp1, FName& OutBone1, UPrimitiveComponent*& OutComp2, FName& OutBone2) const
{
	if (Joints.IsValidIndex(JointHandle)) return false;

	const FIndexedConstraintJoint& Joint = Joints[JointHandle];

	OutComp1 = nullptr;
	OutComp2 = nullptr;
	OutBone1 = NAME_None;
	OutBone2 = NAME_None;

	if (ComponentTable.IsValidIndex(Joint.Component1Index))
	{
		OutComp1 = ComponentTable[Joint.Component1Index].Get();
	}
	if (ComponentTable.IsValidIndex(Joint.Component2Index))
	{
		OutComp2 = ComponentTable[Joint.Component2Index].Get();
	}
	if (BoneNameTable.IsValidIndex(Joint.BoneName1Index))
	{
		OutBone1 = BoneNameTable[Joint.BoneName1Index];
	}
	if (BoneNameTable.IsValidIndex(Joint.BoneName2Index))
	{
		OutBone2 = BoneNameTable[Joint.BoneName2Index];
	}

	return true;
}

bool AZCGearActor::GetConstraintForce(int32 JointHandle, FVector& OutLinearForce, FVector& OutAngularForce)
{
	FConstraintInstance* Instance = GetConstraintInstance(JointHandle);
	if (!Instance)
	{
		return false;
	}
	
	Instance->GetConstraintForce(OutLinearForce, OutAngularForce);
	return true;
}

bool AZCGearActor::IsConstraintBroken(int32 JointHandle)
{
	FConstraintInstance* Instance = GetConstraintInstance(JointHandle);
	return Instance ? Instance->IsBroken() : true;
}

// ===== Limit 계열 =====

void AZCGearActor::SetLinearXLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit)
{
}

void AZCGearActor::SetLinearYLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit)
{
}

void AZCGearActor::SetLinearZLimit(int32 JointHandle, ELinearConstraintMotion Motion, float LinearLimit)
{
}

void AZCGearActor::SetAngularSwing1Limit(int32 JointHandle, EAngularConstraintMotion Motion, float Swing1LimitAngle)
{
}

void AZCGearActor::SetAngularSwing2Limit(int32 JointHandle, EAngularConstraintMotion Motion, float Swing2LimitAngle)
{
}

void AZCGearActor::SetAngularTwistLimit(int32 JointHandle, EAngularConstraintMotion Motion, float TwistLimitAngle)
{
}

void AZCGearActor::SetLinearBreakable(int32 JointHandle, bool bLinearBreakable, float LinearBreakThreshold)
{
}

void AZCGearActor::SetLinearPlasticity(int32 JointHandle, bool bLinearPlasticity, float LinearPlasticityThreshold, EConstraintPlasticityType PlasticityType)
{
}

void AZCGearActor::SetAngularBreakable(int32 JointHandle, bool bAngularBreakable, float AngularBreakThreshold)
{
}

void AZCGearActor::SetAngularPlasticity(int32 JointHandle, bool bAngularPlasticity, float AngularPlasticityThreshold)
{
}

void AZCGearActor::SetContactTransferScale(int32 JointHandle, float ContactTransferScale)
{
}

void AZCGearActor::SetProjectionEnabled(int32 JointHandle, bool bInEnabled)
{
}

void AZCGearActor::SetProjectionParams(int32 JointHandle, float ProjectionLinearAlpha, float ProjectionAngularAlpha, float ProjectionLinearTolerance, float ProjectionAngularTolerance)
{
}

void AZCGearActor::SetLinearPositionDrive(int32 JointHandle, bool bEnableDriveX, bool bEnableDriveY, bool bEnableDriveZ)
{
}

void AZCGearActor::SetLinearVelocityDrive(int32 JointHandle, bool bEnableDriveX, bool bEnableDriveY, bool bEnableDriveZ)
{
}

void AZCGearActor::SetLinearPositionTarget(int32 JointHandle, const FVector& InPosTarget)
{
}

void AZCGearActor::SetLinearVelocityTarget(int32 JointHandle, const FVector& InVelTarget)
{
}

void AZCGearActor::SetLinearDriveParams(int32 JointHandle, float PositionStrength, float VelocityStrength, float InForceLimit)
{
}

void AZCGearActor::SetLinearDriveAccelerationMode(int32 JointHandle, bool bAccelerationMode)
{
}

void AZCGearActor::SetOrientationDriveTwistAndSwing(int32 JointHandle, bool bEnableTwistDrive, bool bEnableSwingDrive)
{
}

void AZCGearActor::SetOrientationDriveSLERP(int32 JointHandle, bool bEnableSLERP)
{
}

void AZCGearActor::SetAngularVelocityDriveTwistAndSwing(int32 JointHandle, bool bEnableTwistDrive, bool bEnableSwingDrive)
{
}

void AZCGearActor::SetAngularVelocityDriveSLERP(int32 JointHandle, bool bEnableSLERP)
{
}

void AZCGearActor::SetAngularDriveMode(int32 JointHandle, EAngularDriveMode::Type DriveMode)
{
}

void AZCGearActor::SetAngularDriveAccelerationMode(int32 JointHandle, bool bAccelerationMode)
{
}

void AZCGearActor::SetAngularOrientationTarget(int32 JointHandle, const FRotator& InPosTarget)
{
}

void AZCGearActor::SetAngularVelocityTarget(int32 JointHandle, const FVector& InVelTarget)
{
}

void AZCGearActor::SetAngularDriveParams(int32 JointHandle, float PositionStrength, float VelocityStrength, float InForceLimit)
{
}

float AZCGearActor::GetCurrentTwist(int32 JointHandle) const
{
	return 0.0f;
}

float AZCGearActor::GetCurrentSwing1(int32 JointHandle) const
{
	return 0.0f;
}

float AZCGearActor::GetCurrentSwing2(int32 JointHandle) const
{
	return 0.0f;
}

int32 AZCGearActor::FindOrAddComponent(UPrimitiveComponent* Component)
{
	if (!Component) return INDEX_NONE;

	for (auto i = 0; i < ComponentTable.Num(); ++i)
	{
		if (ComponentTable[i].Get() == Component) return i;
	}

	return ComponentTable.Add(Component);
}

int32 AZCGearActor::FindOrAddBoneName(FName BoneName)
{
	if (BoneName.IsNone()) return INDEX_NONE;

	for (auto i = 0; i < BoneNameTable.Num(); ++i)
	{
		if (BoneNameTable[i] == BoneName) return i;
	}
	return BoneNameTable.Add(BoneName);
}

FConstraintInstance* AZCGearActor::GetConstraintInstance(int32 JointHandle)
{
	if (!Joints.IsValidIndex(JointHandle)) return nullptr;
	const uint16 Index = Joints[JointHandle].ConstraintIndex;
	return ConstraintInstances.IsValidIndex(Index) ? &ConstraintInstances[Index] : nullptr;
}

const FConstraintInstance* AZCGearActor::GetConstraintInstance(int32 JointHandle) const
{
	if (!Joints.IsValidIndex(JointHandle)) return nullptr;
	const uint16 Index = Joints[JointHandle].ConstraintIndex;
	return ConstraintInstances.IsValidIndex(Index) ? &ConstraintInstances[Index] : nullptr;
}

FBodyInstance* AZCGearActor::GetBodyInstance(int32 JointHandle, bool bFrame1) const
{
	if (!Joints.IsValidIndex(JointHandle)) return nullptr;

	const FIndexedConstraintJoint& Joint = Joints[JointHandle];
	const int32 CompIndex = bFrame1 ? Joint.Component1Index : Joint.Component2Index;

	if (!ComponentTable.IsValidIndex(CompIndex)) return nullptr;

	if (UPrimitiveComponent* PrimComp = ComponentTable[CompIndex].Get())
	{
		const int32 BoneIdx = bFrame1 ? Joint.BoneName1Index : Joint.BoneName2Index;
		const FName BoneName = (BoneIdx != INDEX_NONE && BoneNameTable.IsValidIndex(BoneIdx)) ? BoneNameTable[BoneIdx] : NAME_None;
		return PrimComp->GetBodyInstance(BoneName);
	}

	return nullptr;
}

Chaos::FPhysicsObject* AZCGearActor::GetPhysicsObject(int32 JointHandle, bool bFrame1) const
{
	if (!Joints.IsValidIndex(JointHandle)) return nullptr;

	const FIndexedConstraintJoint& Joint = Joints[JointHandle];
	const int32 CompIndex = bFrame1 ? Joint.Component1Index : Joint.Component2Index;

	if (!ComponentTable.IsValidIndex(CompIndex)) return nullptr;

	if (UPrimitiveComponent* PrimComp = ComponentTable[CompIndex].Get())
	{
		const int32 BoneIdx = bFrame1 ? Joint.BoneName1Index : Joint.BoneName2Index;
		const FName BoneName = (BoneIdx != INDEX_NONE && BoneNameTable.IsValidIndex(BoneIdx)) ? BoneNameTable[BoneIdx] : NAME_None;
		return PrimComp->GetPhysicsObjectByName(BoneName);
	}

	return nullptr;
}

void AZCGearActor::InitConstraintInternal(int32 JointHandle)
{
	if (!Joints.IsValidIndex(JointHandle)) return;

	FIndexedConstraintJoint& Joint = Joints[JointHandle];
	FConstraintInstance* Instance = GetConstraintInstance(JointHandle);
	if (!Instance) return;

	FBodyInstance* Body1 = GetBodyInstance(JointHandle, true);
	FBodyInstance* Body2 = GetBodyInstance(JointHandle, false);

	Chaos::FPhysicsObject* Object1 = (Body1 && Body1->IsValidBodyInstance()) ? Body1->GetPhysicsActor()->GetPhysicsObject() : GetPhysicsObject(JointHandle, true);
	Chaos::FPhysicsObject* Object2 = (Body2 && Body2->IsValidBodyInstance()) ? Body2->GetPhysicsActor()->GetPhysicsObject() : GetPhysicsObject(JointHandle, false);

	if (Object1 || Object2)
	{
		const float ConstraintScale = 1.f; // 필요하면 액터 스케일 반영
		Instance->InitConstraint(Object1, Object2, ConstraintScale, nullptr, FOnConstraintBroken());
		Joint.bInitialized = true;
	}
}

void AZCGearActor::TermConstraintInternal(int32 JointHandle)
{
	FConstraintInstance* Instance = GetConstraintInstance(JointHandle);
	if (Instance)
	{
		Instance->TermConstraint();
	}

	if (Joints.IsValidIndex(JointHandle))
	{
		Joints[JointHandle].bInitialized = false;
	}
}
