// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Reaction/ZCHitReactionComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"

#include "Development/ZCLogger.h"

// Sets default values for this component's properties
UZCHitReactionComponent::UZCHitReactionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// ...

	/*========================================================================*/
	MotionWarpingDist.Add(EHitStrength::Explosion,	400.0f);
	MotionWarpingDist.Add(EHitStrength::Heavy,		250.0f);
	MotionWarpingDist.Add(EHitStrength::Medium,		150.0f);

	/*========================================================================*/
	PhysicsReationStrength.Add(EHitStrength::Explosion, 10000.0f);
	PhysicsReationStrength.Add(EHitStrength::Heavy,		7000.0f);
	PhysicsReationStrength.Add(EHitStrength::Medium,	5000.0f);
	PhysicsReationStrength.Add(EHitStrength::Light,		3000.0f);

	/*========================================================================*/
	HitPosMap.Add(FName(TEXT("head")), EHitPosition::Head);
	HitPosMap.Add(FName(TEXT("neck_01")), EHitPosition::Head);
	HitPosMap.Add(FName(TEXT("neck_02")), EHitPosition::Head);

	HitPosMap.Add(FName(TEXT("spine_05")), EHitPosition::Chest);
	HitPosMap.Add(FName(TEXT("spine_04")), EHitPosition::Chest);

	HitPosMap.Add(FName(TEXT("spine_03")), EHitPosition::Abdomen);
	HitPosMap.Add(FName(TEXT("spine_02")), EHitPosition::Abdomen);

	HitPosMap.Add(FName(TEXT("spine_01")), EHitPosition::Pelvis);

	HitPosMap.Add(FName(TEXT("clavicle_l")), EHitPosition::LeftShoulder);
	HitPosMap.Add(FName(TEXT("clavicle_r")), EHitPosition::RightShoulder);

	HitPosMap.Add(FName(TEXT("pelvis")), EHitPosition::Pelvis);

	HitPosMap.Add(FName(TEXT("upperarm_l")), EHitPosition::LeftShoulder);
	HitPosMap.Add(FName(TEXT("lowerarm_l")), EHitPosition::LeftArm);
	HitPosMap.Add(FName(TEXT("hand_l")), EHitPosition::LeftArm);

	HitPosMap.Add(FName(TEXT("upperarm_r")), EHitPosition::RightShoulder);
	HitPosMap.Add(FName(TEXT("lowerarm_r")), EHitPosition::RightArm);
	HitPosMap.Add(FName(TEXT("hand_r")), EHitPosition::RightArm);

	HitPosMap.Add(FName(TEXT("thigh_l")), EHitPosition::LeftThigh);
	HitPosMap.Add(FName(TEXT("calf_l")), EHitPosition::LeftCalf);
	HitPosMap.Add(FName(TEXT("foot_l")), EHitPosition::LeftCalf);

	HitPosMap.Add(FName(TEXT("thigh_r")), EHitPosition::RightThigh);
	HitPosMap.Add(FName(TEXT("calf_r")), EHitPosition::RightCalf);
	HitPosMap.Add(FName(TEXT("foot_r")), EHitPosition::RightCalf);

	HitPosMap.Add(FName(TEXT("Hips")), EHitPosition::Pelvis);

	HitPosMap.Add(FName(TEXT("LeftUpLeg")), EHitPosition::LeftThigh);
	HitPosMap.Add(FName(TEXT("LeftLeg")), EHitPosition::LeftCalf);
	HitPosMap.Add(FName(TEXT("LeftFoot")), EHitPosition::LeftCalf);

	HitPosMap.Add(FName(TEXT("RightUpLeg")), EHitPosition::RightThigh);
	HitPosMap.Add(FName(TEXT("RightLeg")), EHitPosition::RightCalf);
	HitPosMap.Add(FName(TEXT("RightFoot")), EHitPosition::RightCalf);

	HitPosMap.Add(FName(TEXT("Spine")), EHitPosition::Abdomen);
	HitPosMap.Add(FName(TEXT("Spine1")), EHitPosition::Abdomen);
	HitPosMap.Add(FName(TEXT("Spine2")), EHitPosition::Chest);
	
	HitPosMap.Add(FName(TEXT("LeftShoulder")), EHitPosition::LeftShoulder);
	HitPosMap.Add(FName(TEXT("LeftArm")), EHitPosition::LeftArm);
	HitPosMap.Add(FName(TEXT("LeftForeArm")), EHitPosition::LeftArm);
	HitPosMap.Add(FName(TEXT("LeftHand")), EHitPosition::LeftArm);

	HitPosMap.Add(FName(TEXT("Neck")), EHitPosition::Head);

	HitPosMap.Add(FName(TEXT("RightShoulder")), EHitPosition::RightShoulder);
	HitPosMap.Add(FName(TEXT("RightArm")), EHitPosition::RightShoulder);
	HitPosMap.Add(FName(TEXT("RightForeArm")), EHitPosition::RightArm);
	HitPosMap.Add(FName(TEXT("RightHand")), EHitPosition::RightArm);


	/*========================================================================*/
	CenterOfMassBone = FName(TEXT("pelvis"));
	HitBoneName = FName(TEXT("None"));

	PhysicsControlData.LinearStrength = 3.0f;
	PhysicsControlData.LinearDampingRatio = 1.0f;
	PhysicsControlData.LinearExtraDamping = 0.0f;
	PhysicsControlData.MaxForce = 0.0f;
	PhysicsControlData.AngularStrength = 1.5f;
	PhysicsControlData.AngularDampingRatio = 1.0f;
	PhysicsControlData.AngularExtraDamping = 0.5f;
	PhysicsControlData.MaxTorque = 0.0f;
	PhysicsControlData.LinearTargetVelocityMultiplier = 1.0f;
	PhysicsControlData.AngularTargetVelocityMultiplier = 1.0f;
	PhysicsControlData.SkeletalAnimationVelocityMultiplier = 1.0f;
	PhysicsControlData.CustomControlPoint = FVector::ZeroVector;
	PhysicsControlData.bEnabled = true;
	PhysicsControlData.bUseCustomControlPoint = false;
	PhysicsControlData.bUseSkeletalAnimation = true;
	PhysicsControlData.bDisableCollision = false;
	PhysicsControlData.bOnlyControlChildObject = false;

	BodyModifierData.MovementType = EPhysicsMovementType::Simulated;
	BodyModifierData.CollisionType = ECollisionEnabled::QueryAndPhysics;
	BodyModifierData.GravityMultiplier = 0.0f;
	BodyModifierData.PhysicsBlendWeight = 0.0f;
	BodyModifierData.bUseSkeletalAnimation = true;
	BodyModifierData.bUpdateKinematicFromSimulation = true;
}

void UZCHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePhysics(DeltaTime);
}

void UZCHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ActorAnimInstance)
	{
		ActorAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UZCHitReactionComponent::NotifyBegin);
		ActorAnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UZCHitReactionComponent::NotifyEnd);
	}

	Super::EndPlay(EndPlayReason);
}

void UZCHitReactionComponent::Init(UMotionWarpingComponent& InMotionWarpingComponent, UPhysicsControlComponent& InPhysicsControlComponent, USkeletalMeshComponent& InMesh)
{
	MotionWarpingComponent = &InMotionWarpingComponent;
	PhysicsControlComponent = &InPhysicsControlComponent;
	MeshComponent = &InMesh;

	MeshAsset = MeshComponent->GetSkeletalMeshAsset();

	check(MotionWarpingComponent);
	check(PhysicsControlComponent);
	check(MeshComponent);

	InitPhysicsControl();

	ActorAnimInstance = MeshComponent->GetAnimInstance();
	if (ActorAnimInstance)
	{
		ActorAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UZCHitReactionComponent::NotifyBegin);
		ActorAnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UZCHitReactionComponent::NotifyEnd);
	}

	OriginalMeshLocation = MeshComponent->GetRelativeLocation();
}

void UZCHitReactionComponent::UpdatePhysics(float DeltaTime)
{
	if (DisablePhysicsParts.Contains(HitPosition)) return;

	if (StartPhysicsReaction)
	{
		CurrentPhysicsReactionDuration += DeltaTime;

		if (CurrentPhysicsReactionDuration < PhysicsReactionDuration)
		{
			PhysicsControlComponent->SetBodyModifiersInSetPhysicsBlendWeight(FName(TEXT("All")), FMath::Clamp(((CurrentPhysicsReactionDuration / PhysicsReactionDuration) - 1.0f), 0.0f, 1.0f));
		}
		else
		{
			StartPhysicsReaction = false;

			PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("All")), EPhysicsMovementType::Kinematic);
			PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("LowerBody")), EPhysicsMovementType::Static);
		}
	}
	else
	{
		CurrentPhysicsReactionDuration = 0.0f;
	}
}

void UZCHitReactionComponent::UpdateMontionWarping()
{
	FVector TargetLocation;
	FRotator TargetRotation;

	FVector ActorForward = (-GetOwner()->GetActorForwardVector()).GetSafeNormal2D();
	FVector HitForward = (-HitDir).GetSafeNormal2D();

	TargetLocation = GetOwner()->GetActorLocation() + (HitDir * MotionWarpingDist[HitStrength]);

	TargetRotation = UKismetMathLibrary::FindLookAtRotation(ActorForward, HitForward);

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("HitReaction"), TargetLocation, TargetRotation);
}

void UZCHitReactionComponent::PerformHitReaction(const AActor* HitCauser, const FHitResult& HitResult, EHitStrength InputHitStrength, bool UseCauseDeath, bool UseHitCurserDirection)
{
	UAnimMontage* Montage = StartHitReaction(HitCauser, HitResult, InputHitStrength, UseCauseDeath, UseHitCurserDirection);
	PerformMontage(Montage);
	PerformPhysicsReaction();
}

UAnimMontage* UZCHitReactionComponent::StartHitReaction(const AActor* HitCauser, const FHitResult& HitResult, EHitStrength InputHitStrength, bool UseCauseDeath, bool UseHitCurserDirection)
{
	bUseHitCurserDirection = UseHitCurserDirection;
	HitStrength = InputHitStrength;
	bCauseDeath = UseCauseDeath;

	if (MeshComponent)
	{
		HitBoneName = MeshComponent->FindClosestBone(HitResult.ImpactPoint);
		HitDir = (HitResult.TraceEnd - HitResult.TraceStart).GetSafeNormal();

		HitDirection = GetHitDirection(HitResult, HitCauser);
		HitPosition = GetHitPosition(HitBoneName);

		//UZCLogger::Warning(TEXT("Hit Reaction: DamagedActor : {0}, HitBoneName: {1}, HitPosition: {2}, HitDirection: {3}, CurrentHitStrength: {4}"), HitCauser, HitBoneName, HitPosition, HitDirection, HitStrength);

		FChooserEvaluationContext Context;
		Context.AddObjectParam(this);

		FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(HitReactionChooserTable);

		TArray<TSoftObjectPtr<UObject>> RawResults = UChooserFunctionLibrary::EvaluateObjectChooserBaseMultiSoft(Context, ChooserStruct, UAnimMontage::StaticClass(), false);

		UAnimMontage* SelectedMontage = nullptr;

		if (RawResults.Num() == 0) return nullptr;

		int32 Index = bCauseDeath ? 0 : FMath::RandRange(0, RawResults.Num() - 1);

		UObject* LoadedObject = RawResults[Index].LoadSynchronous();
		SelectedMontage = Cast<UAnimMontage>(LoadedObject);

		return SelectedMontage;
	}

	return nullptr;
}

void UZCHitReactionComponent::PerformMontage(UAnimMontage* Montage)
{
	if (!Montage) return;
	if (bIsDying || bIsDeath) return;

	ActorAnimInstance->Montage_Stop(0.0f);

	if (HitStrength == EHitStrength::Light)
	{
		ActorAnimInstance->Montage_StopGroupByName(0.0f, FName(TEXT("AdditiveGroup")));
		ActorAnimInstance->Montage_Play(Montage, 1.5f);
	}
	else if (bCauseDeath || HitStrength == EHitStrength::Explosion || bCanPlayNewMontage)
	{
		if (bCauseDeath) bIsDying = true;

		ActorAnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		UpdateMontionWarping();
	}
}

void UZCHitReactionComponent::PerformPhysicsReaction()
{
	StartPhysicsReaction = true;

	PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("All")), EPhysicsMovementType::Simulated);
	PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("LowerBody")), EPhysicsMovementType::Static);

	GetWorld()->GetTimerManager().SetTimer(PhysicsReactionTimerHandle, [this]() {
		 if (MeshComponent->IsSimulatingPhysics(HitBoneName))
		 {
			MeshComponent->AddImpulse(HitDir * PhysicsReationStrength[HitStrength], HitBoneName, true);
		 }
	}, 0.05, false);

}

void UZCHitReactionComponent::NotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName(TEXT("DeadNotify")))
	{
		UZCLogger::Warning(TEXT("Hit Reaction: DeadNotify Triggered"));
		ActorAnimInstance->SavePoseSnapshot(FName(TEXT("HitReactionDead")));
	}
	else if (NotifyName == FName(TEXT("StopPlayNewMontage")))
	{
		bCanPlayNewMontage = false;

		MeshComponent->SetRelativeLocation(MeshLocationOffset[HitStrength]);
	}
}

void UZCHitReactionComponent::NotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName(TEXT("DeadNotify")))
	{
		if (bCauseDeath)
		{
			bIsDeath = true;
		}
	}
	else if (NotifyName == FName(TEXT("StopPlayNewMontage")))
	{
		bCanPlayNewMontage = true;

		if (!bCauseDeath)
		{
			MeshComponent->SetRelativeLocation(OriginalMeshLocation);

		}
	}
}

void UZCHitReactionComponent::InitPhysicsControl()
{
	PhysicsControlComponent->CreateControlsFromSkeletalMeshBelow(MeshComponent, CenterOfMassBone, false, EPhysicsControlType::WorldSpace, PhysicsControlData, FName(TEXT("All")));

	TArray<FName> BoneNames = PhysicsControlComponent->CreateBodyModifiersFromSkeletalMeshBelow(MeshComponent, CenterOfMassBone, false, FName(TEXT("All")), BodyModifierData);

	for (auto& BoneName : BoneNames)
	{
		if (UKismetStringLibrary::Contains(BoneName.ToString(), TEXT("foot")))
		{
			TArray<FName> BodyModifierNames;

			BodyModifierNames.Add(BoneName);
			BodyModifierNames.Add(MeshComponent->GetParentBone(BoneName));

			FPhysicsControlNames LowerBodySet;
			PhysicsControlComponent->AddBodyModifiersToSet(LowerBodySet, BodyModifierNames, FName(TEXT("LowerBody")));
		}
	}

	PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("All")), EPhysicsMovementType::Kinematic);
	PhysicsControlComponent->SetBodyModifiersInSetMovementType(FName(TEXT("LowerBody")), EPhysicsMovementType::Static);
}

EZCHitDirection UZCHitReactionComponent::GetHitDirection(const FHitResult& HitResult, const AActor* HitCauser) const
{
	FVector ImpactNormal;

	const AActor* HitActor = HitResult.GetActor();

	if (bUseHitCurserDirection && HitCauser)
	{
		ImpactNormal = (HitCauser->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();
	}
	else
	{
		ImpactNormal = (HitResult.TraceStart - HitResult.TraceEnd).GetSafeNormal();
	}

	const float Forward		= FVector::DotProduct(ImpactNormal, HitActor->GetActorForwardVector());
	const float Right		= FVector::DotProduct(ImpactNormal, HitActor->GetActorRightVector());
	const float Up			= FVector::DotProduct(ImpactNormal, HitActor->GetActorUpVector());

	const float AbsForward	= FMath::Abs(Forward);
	const float AbsRight	= FMath::Abs(Right);
	const float AbsUp		= FMath::Abs(Up);

	if (AbsForward > AbsRight && AbsForward > AbsUp)
	{
		return Forward > 0.0f ? EZCHitDirection::Front : EZCHitDirection::Back;
	}
	else if (AbsRight > AbsForward && AbsRight > AbsUp)
	{
		return Right > 0.0f ? EZCHitDirection::Right : EZCHitDirection::Left;
	}
	else
	{
		return Up > 0.0f ? EZCHitDirection::Top : EZCHitDirection::Bottom;
	}
}

EHitPosition UZCHitReactionComponent::GetHitPosition(const FName& BoneName) const
{
	return HitPosMap.Contains(BoneName) ? HitPosMap[BoneName] : EHitPosition::Other;
}

