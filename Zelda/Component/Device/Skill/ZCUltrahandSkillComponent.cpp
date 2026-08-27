// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Device/Skill/ZCUltrahandSkillComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Engine/OverlapResult.h"
#include "Components/CapsuleComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "Player/ZCHUDLocalPlayerSubsystem.h"
#include "World/Subsystem/ZCUltrahandWorldSubsystem.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Actor/ZCActor.h"
#include "Actor/Util/ZCUtilActor.h"
#include "Player/ZCPlayerController.h"
#include "Physics/ZCCollision.h"
#include "Component/Device/ZCDeviceEnum.h"
#include "Development/ZCLogger.h"
#include "Development/ZCDebug.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCUltrahandSkillComponent)

UZCUltrahandSkillComponent::UZCUltrahandSkillComponent()
{
	DeviceSkillCollisionChannel = Zelda::Channel::Ultrahand;
}

void UZCUltrahandSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	UltrahandSubsystem = GetWorld()->GetSubsystem<UZCUltrahandWorldSubsystem>();

	SpawnGluePoint();
	SpawnGizmo();
}

void UZCUltrahandSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentState)
	{
	case EUltraHandState::Manipulation:
		UpdateManipulation();
		[[fallthrough]];

	case EUltraHandState::SelectedSingle:
	case EUltraHandState::SelectedMulti:
		// 잡은 오브젝트의 위치 동기화
		UpdateGrabbedObjectPosition();

		// 잡은 오브젝트와 가장 가까운 다른 오브젝트를 검색 후 Snab 로직 실행
		UpdateSnabToNearestActor();
		[[fallthrough]];

	case EUltraHandState::Find:
		// 울트라 핸드와 상호작용 가능한 오브젝트 시각화
		UpdateInteractableActors();
		if (CurrentState == EUltraHandState::Find)
		{
			UpdateCrosshairTracking();
		}
		else
		{
			HUDSubsystem->HideCrosshair();
		}
		break;

	default: break;
	}

}

void UZCUltrahandSkillComponent::SetupInputComponent(UEnhancedInputComponent* InputComponent)
{
	Super::SetupInputComponent(InputComponent);

	InputComponent->BindAction(DetachObjectAction, ETriggerEvent::Triggered, this, &UZCUltrahandSkillComponent::DetachObject);
	InputComponent->BindAction(MoveForwardBackwardAction, ETriggerEvent::Triggered, this, &UZCUltrahandSkillComponent::MoveForwardBackward);

	InputComponent->BindAction(ManipulationModeAction, ETriggerEvent::Started, this, &UZCUltrahandSkillComponent::StartManipulation);
	InputComponent->BindAction(ManipulationModeAction, ETriggerEvent::Completed, this, &UZCUltrahandSkillComponent::StopManipulation);

	InputComponent->BindAction(ManipulationAction, ETriggerEvent::Triggered, this, &UZCUltrahandSkillComponent::Manipulation);
	InputComponent->BindAction(ManipulationResetRotationAction, ETriggerEvent::Triggered, this, &UZCUltrahandSkillComponent::ManipulationResetRotation);
}

void UZCUltrahandSkillComponent::SetPhysicsHandleComponent(UPhysicsHandleComponent* NewGraper)
{
	GrabHandle = NewGraper;

	GrabHandle->LinearStiffness = 100000.0f;
	GrabHandle->AngularStiffness = 100000.0f;
	GrabHandle->LinearDamping = 10000.0f;
	GrabHandle->AngularDamping = 10000.0f;
	GrabHandle->InterpolationSpeed = 50.0f;
}

void UZCUltrahandSkillComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	HUDSubsystem->ShowCrosshair();

	UpdateState(EUltraHandState::Find);
}

void UZCUltrahandSkillComponent::Deactivate()
{
	UpdateState(EUltraHandState::None);
	
	HUDSubsystem->HideCrosshair();

	DeselectCurrentActor();

	for (AZCActor* Actor : InteractableActors)
	{
		if (Actor)
		{
			Actor->ReleaseOverlayState(EOverlayState::UltraHand);
		}
	}

	InteractableActors.Empty();

	if (GrabHandle && GrabHandle->GrabbedComponent)
	{
		GrabHandle->ReleaseComponent();
	}

	SetGluePointVisible(false);

	SelectedActorGroup.Empty();
	AssembleCandidates.Empty();

	Super::Deactivate();
}

void UZCUltrahandSkillComponent::AddAssembleCandidate(AZCActor* OwnerActor, AZCActor* AddActor)
{
	if (SelectedActorGroup.Contains(OwnerActor) && SelectedActorGroup.Contains(AddActor)) return;
	AssembleCandidates.Add({OwnerActor, AddActor});
}

void UZCUltrahandSkillComponent::RemoveAssembleCandidate(AZCActor* OwnerActor, AZCActor* RemoveActor)
{	
	for (int32 Index = AssembleCandidates.Num() - 1; Index >= 0; --Index)
	{
		const auto &[Key, Value] = AssembleCandidates[Index];

		if (Key == OwnerActor && Value == RemoveActor)
		{
			AssembleCandidates.RemoveAtSwap(Index, EAllowShrinking::No);
		}
	}
}

void UZCUltrahandSkillComponent::SetHUDSubsystem(UZCHUDLocalPlayerSubsystem* NewLocalSub)
{
	Super::SetHUDSubsystem(NewLocalSub);

	HUDSubsystem->AddSkillButtonContext(SkillType, EFaceButtonType::BButton, FText::FromString(TEXT("붙이기")));
}

void UZCUltrahandSkillComponent::UpdateState(EUltraHandState NewState)
{
	CurrentState = NewState;

	HUDSubsystem->ShowSkillIndicator(SkillType, static_cast<int32>(CurrentState));
}

void UZCUltrahandSkillComponent::Action()
{
	switch (CurrentState)
	{
	case EUltraHandState::Find:
		Action_Find_Internal();
		break;
	case EUltraHandState::SelectedSingle:
	case EUltraHandState::SelectedMulti:
		Action_Select_Internal();
		break;
	case EUltraHandState::Manipulation:
		break;
	}
}

void UZCUltrahandSkillComponent::Cancel()
{
	// 테스트 코드
	//UltrahandSubsystem->DetachActor(SelectedActor);

	Deactivate();
}

void UZCUltrahandSkillComponent::DetachObject()
{
	if (!SelectedActor || SelectedActorGroup.IsEmpty()) return;

	UltrahandSubsystem->DetachActor(SelectedActor);

	Deactivate();
}

void UZCUltrahandSkillComponent::MoveForwardBackward(const FInputActionValue& Value)
{
	if (CurrentState != EUltraHandState::SelectedSingle && CurrentState != EUltraHandState::SelectedMulti) return;

	const float Direction = Value.Get<float>();

	float NewDistance = InitialGrabDistance + (Direction * PushPullStep);

	InitialGrabDistance = FMath::Clamp(NewDistance, MinGrabDistance, CatchMaxDistance);
}

void UZCUltrahandSkillComponent::StartManipulation(const FInputActionValue& Value)
{
	if (!SelectedActor) return;
	UpdateState(EUltraHandState::Manipulation);

	SetGizmoVisible(true);
}

void UZCUltrahandSkillComponent::StopManipulation(const FInputActionValue& Value)
{
	SelectedActorGroup.Num() > 1 ? UpdateState(EUltraHandState::SelectedMulti) : UpdateState(EUltraHandState::SelectedSingle);

	SetGizmoVisible(false);
}

void UZCUltrahandSkillComponent::Manipulation(const FInputActionValue& Value)
{
	if (!SelectedActor) return;
	const FVector2D Direction = Value.Get<FVector2D>();

	FVector RotationAxis = FVector::ZeroVector;

	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();

	// 좌우 (Horizontal) : 월드 Z축 기준 회전
	if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
	{
		RotationAxis = FVector::UpVector; // 월드 Z축
		AngleDeg = (Direction.X > 0.0f) ? 90.0f : -90.0f;
	}
	// 상하 (Vertical) : 카메라 Right 벡터 기준 회전 (Pitch)
	else
	{
		RotationAxis = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
		AngleDeg = (Direction.Y > 0.0f) ? 90.0f : -90.0f;
	}

	FQuat DeltaRotation(RotationAxis, FMath::DegreesToRadians(AngleDeg));

	TargetWorldRotation = DeltaRotation * TargetWorldRotation;

	TargetWorldRotation.Normalize();
}

void UZCUltrahandSkillComponent::ManipulationResetRotation()
{
	if (!SelectedActor) return;

	FRotator DefaultRotation = SelectedActor->GetClass()->GetDefaultObject<AZCActor>()->GetActorRotation();

	TargetWorldRotation = DefaultRotation.Quaternion();
}

void UZCUltrahandSkillComponent::Action_Find_Internal()
{
	if (!TargetedActor) return;

	AZCActor* NewTargetActor = nullptr;
	FHitResult HitResult;

	if (!TraceObjectFromCamera(CatchMaxDistance, HitResult, &NewTargetActor)) return;

	DeselectCurrentActor();

	if (NewTargetActor)
	{
		SelectActor(NewTargetActor);
		GrabActorAtHitPoint(HitResult);

		SelectedActorGroup.Num() > 1 ? UpdateState(EUltraHandState::SelectedMulti) : UpdateState(EUltraHandState::SelectedSingle);
	}
}

void UZCUltrahandSkillComponent::Action_Select_Internal()
{
	if (!SelectedActor) return;

	if (SnapTargetActor && GlueActorClass)
	{
		FVector SpawnLoc = (SnapSourceLocation + SnapTargetLocation) * 0.5f;
		FRotator SpawnRot = FRotator::ZeroRotator;

		AZCGlueActor* GlueActor = GetWorld()->SpawnActor<AZCGlueActor>(GlueActorClass, SpawnLoc, SpawnRot);
		GlueActor->PerformAttachment(UltrahandSubsystem, SnapSourceActor, SnapTargetActor, SnapSourceComponent.Get(), SnapTargetComponent.Get(), SnapSourceLocation, SnapTargetLocation);

		Deactivate();

	}
}

bool UZCUltrahandSkillComponent::TraceObjectFromCamera(float Distance, FHitResult& OutHitResult, AZCActor** OutActor) const
{
	*OutActor = nullptr;
	FVector CameraLocation;
	FRotator CameraRotation;

	if (!PC) return false;

	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector Start = CameraLocation;

	FVector End = Start + (CameraRotation.Vector() * Distance);

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(CatchDetectionRadius);

	const bool bHit = DeviceSweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, SphereShape);

	if (bHit)
	{
		*OutActor = Cast<AZCActor>(OutHitResult.GetActor());
	}

	return bHit && (*OutActor != nullptr);
}

void UZCUltrahandSkillComponent::UpdateGrabbedObjectPosition()
{
	if (!SelectedActor || !GrabHandle) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->PlayerCameraManager->GetCameraViewPoint(CameraLocation, CameraRotation);

	const FVector TargetLocation = CalculateTargetGrabLocation();

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float CurrentCameraYaw = CameraRotation.Yaw;
	float DeltaYaw = FMath::FindDeltaAngleDegrees(LastCameraYaw, CurrentCameraYaw);

	// 변화량을 쿼터니언으로 변환 (World Z축 기준 회전)
	FQuat YawDeltaRot(FVector::UpVector, FMath::DegreesToRadians(DeltaYaw));

	TargetWorldRotation = YawDeltaRot * TargetWorldRotation;
	SmoothedTargetWorldRotation = YawDeltaRot * SmoothedTargetWorldRotation;

	LastCameraYaw = CurrentCameraYaw;

	SmoothedTargetWorldRotation = FMath::QInterpTo(SmoothedTargetWorldRotation, TargetWorldRotation, DeltaTime, RotationInterpSpeed);
	SmoothedTargetWorldRotation.Normalize();

	// 물리 핸들에 적용
	GrabHandle->SetTargetLocationAndRotation(TargetLocation, SmoothedTargetWorldRotation.Rotator());

	// VFX 처리
	FVector WorldBeamEnd = SelectedActor->GetActorLocation();
	FVector LocalBeamEnd = PlayerVFXComponent->GetComponentTransform().InverseTransformPosition(WorldBeamEnd);
	PlayerVFXComponent->SetVariableVec3(FName(TEXT("User.beamEnd")), LocalBeamEnd);

}

void UZCUltrahandSkillComponent::UpdateInteractableActors()
{
	TSet<AZCActor*> DetectedActors;
	TArray<FOverlapResult> OverlapResults;

	const FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractionOverlayRange);

#if !UE_BUILD_SHIPPING
	using namespace Zelda::Debug::Ultrahand;
	if (bDrawDebugAll || bDrawDevice)
	{
		DrawDebugSphere(GetWorld(), PlayerLocation, InteractionOverlayRange, 16, FColor::Green, false, 0.0f, 0, 2.0f);
	}

#endif // !UE_BUILD_SHIPPING

	if (DeviceOverlapMultiByChannel(OverlapResults, PlayerLocation, FQuat::Identity, SphereShape))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AZCActor* Actor = Cast<AZCActor>(Result.GetActor()))
			{
				DetectedActors.Add(Actor);
			}
		}
	}

	UpdateActorOverlayStates(DetectedActors, InteractableActors);
	InteractableActors = DetectedActors;
}

void UZCUltrahandSkillComponent::UpdateSnabToNearestActor()
{
	float MinDistanceSquared = FLT_MAX;
	int32 BestCandidateIndex = -1;

	UPrimitiveComponent* BestSourceComp = nullptr;
	UPrimitiveComponent* BestTargetComp = nullptr;

	FVector FinalClosestOnSource = FVector::ZeroVector;
	FVector FinalClosestOnTarget = FVector::ZeroVector;

	// 상호작용 가능한 최대 거리의 제곱값 미리 계산
	const float MinInteractDistSq = MinInteractionDistance * MinInteractionDistance;

	TInlineComponentArray<UMeshComponent*, 8> SourceComponents;
	TInlineComponentArray<UMeshComponent*, 8> TargetComponents;

	for (int32 i = 0; i < AssembleCandidates.Num(); ++i)
	{
		AZCActor* SourceActor = AssembleCandidates[i].Key;
		AZCActor* TargetActor = AssembleCandidates[i].Value;

		if (!SourceActor || !TargetActor) continue;

		SourceComponents.Reset();
		SourceActor->GetComponents(SourceComponents);

		TargetComponents.Reset();
		TargetActor->GetComponents(TargetComponents);

		for (UMeshComponent* Source : SourceComponents)
		{
			if (Source->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics) continue;

			const FBoxSphereBounds& SourceBounds = Source->Bounds;

			for (UMeshComponent* Target : TargetComponents)
			{
				if (Target->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics) continue;

				const FBoxSphereBounds& TargetBounds = Target->Bounds;

				// Broad Phase : Bounding Sphere를 이용한 빠른 필터링
				float CenterDist = FVector::Dist(SourceBounds.Origin, TargetBounds.Origin);
				// 대략적인 거리
				float ApproxMinDist = FMath::Max(0.0f, CenterDist - (SourceBounds.SphereRadius + TargetBounds.SphereRadius));

				if (FMath::Square(ApproxMinDist) > FMath::Min(MinDistanceSquared, MinInteractDistSq)) continue;

				// Narrow Phase : 실제 정밀 거리 계산
				const FVector SourceLocation = Source->GetComponentLocation();
				FVector PointOnTarget, PointOnSource, RefinedPointOnTarget;

				if (!Target->GetClosestPointOnCollision(SourceLocation, PointOnTarget)) continue;
				if (!Source->GetClosestPointOnCollision(PointOnTarget, PointOnSource)) continue;
				if (!Target->GetClosestPointOnCollision(PointOnSource, RefinedPointOnTarget)) continue;

				const float DistSq = FVector::DistSquared(PointOnSource, RefinedPointOnTarget);

				if (DistSq < MinDistanceSquared)
				{
					MinDistanceSquared = DistSq;
					BestCandidateIndex = i;
					BestSourceComp = Source;
					BestTargetComp = Target;
					FinalClosestOnSource = PointOnSource;
					FinalClosestOnTarget = RefinedPointOnTarget;
				}
			}
		}
	}

	// 최종적으로 유효한 거리 내에 있는 컴포넌트 쌍을 찾았을 경우
	if (BestCandidateIndex != -1 && MinDistanceSquared <= MinInteractDistSq)
	{
		SnapSourceActor = AssembleCandidates[BestCandidateIndex].Key;
		SnapTargetActor = AssembleCandidates[BestCandidateIndex].Value;

		// 찾은 가장 가까운 컴포넌트들을 멤버 변수에 저장
		SnapSourceComponent = BestSourceComp;
		SnapTargetComponent = BestTargetComp;

		const float SnapSocketDistSq = SnapSocketMaxDistance * SnapSocketMaxDistance;

		// 소켓 스냅을 처리하는 람다 함수
		auto ApplySocketSnap = [&](UPrimitiveComponent* InComponent, FVector& InOutPoint)
			{
				if (!InComponent) return;

				TArray<FName> SocketNames = InComponent->GetAllSocketNames();
				if (SocketNames.IsEmpty()) return;

				float BestSocketDistSq = SnapSocketDistSq;
				FVector BestSocketPos = FVector::ZeroVector;
				bool bFoundSocket = false;

				for (const FName& SocketName : SocketNames)
				{
					const FVector SocketPos = InComponent->GetSocketLocation(SocketName);
					const float DistSq = FVector::DistSquared(InOutPoint, SocketPos);

					if (DistSq < BestSocketDistSq)
					{
						BestSocketDistSq = DistSq;
						BestSocketPos = SocketPos;
						bFoundSocket = true;
					}
				}
				if (bFoundSocket)
				{
					InOutPoint = BestSocketPos;
				}
			};

		// 소스(잡고 있는 물체)의 접점 보정
		ApplySocketSnap(BestSourceComp, FinalClosestOnSource);
		// 타겟(붙일 대상 물체)의 접점 보정
		ApplySocketSnap(BestTargetComp, FinalClosestOnTarget);

		SnapSourceLocation = FinalClosestOnSource;
		SnapTargetLocation = FinalClosestOnTarget;

		UpdateGluePointTransform(FinalClosestOnSource, FinalClosestOnTarget);
		SetGluePointVisible(true);

		if (HUDSubsystem)
		{
			HUDSubsystem->ShowSkillButtonEvent(SkillType, EFaceButtonType::BButton);
		}

#if !UE_BUILD_SHIPPING
		using namespace Zelda::Debug::Ultrahand;
		if (bDrawDebugAll || bDrawDevice)
		{
			DrawDebugSphere(GetWorld(), FinalClosestOnSource, 3.0f, 16, FColor::Blue, false, 0.0f, 0, 2.0f);
			DrawDebugSphere(GetWorld(), FinalClosestOnTarget, 3.0f, 16, FColor::Red, false, 0.0f, 0, 2.0f);
			DrawDebugLine(GetWorld(), FinalClosestOnSource, FinalClosestOnTarget, FColor::Green, false, 0.0f, 0, 1.5f);
		}
#endif
	}
	else
	{
		// 조건 불만족: 모든 스냅 정보 초기화
		SnapSourceActor = nullptr;
		SnapTargetActor = nullptr;
		SnapSourceComponent = nullptr;
		SnapTargetComponent = nullptr;

		SetGluePointVisible(false);

		if (HUDSubsystem)
		{
			HUDSubsystem->HideSkillButtonEvent(SkillType, EFaceButtonType::BButton);
		}
	}
}

void UZCUltrahandSkillComponent::UpdateCrosshairTracking()
{
	AZCActor* HitActor = nullptr;
	FHitResult HitResult;

	// 카메라 방향으로 트레이스 수행
	bool bHit = TraceObjectFromCamera(CatchMaxDistance, HitResult, &HitActor);

	// 감지된 액터가 있고, 기존 타겟과 다르다면 갱신
	if (bHit && HitActor)
	{
		TargetedActor = HitActor;
		// 락온 상태 활성화 (십자선이 붉게 변하거나 타겟 표시)
		HUDSubsystem->SetCrosshairLockOn(true);
	}
	else
	{
		TargetedActor = nullptr;
		HUDSubsystem->SetCrosshairLockOn(false);
	}
}

void UZCUltrahandSkillComponent::UpdateManipulation()
{
	if (!SelectedActor) return;

	UpdateGizmoTransform(SelectedActor->GetActorLocation());
}

void UZCUltrahandSkillComponent::SelectActor(AZCActor* Actor)
{
	if (!Actor) return;

	SelectedActor = Actor;

	SelectedActorGroup = UltrahandSubsystem->GetConnectedGroupSet(SelectedActor);

	for (const auto& Group : SelectedActorGroup)
	{
		Group->SetGrabState(true, this);
		Group->RequestOverlayState(EOverlayState::SelectUltraHand);
		//UZCLogger::Warning(TEXT("액터들 : {0}"), Group);
	}

}

void UZCUltrahandSkillComponent::DeselectCurrentActor()
{
	// 그룹 전체 멤버의 Grab 상태 및 오버레이 해제
	for (AZCActor* Actor : SelectedActorGroup)
	{
		if (Actor)
		{
			Actor->ReleaseOverlayState(EOverlayState::SelectUltraHand);
			Actor->SetGrabState(false);
		}
	}
	SelectedActorGroup.Empty();
	AssembleCandidates.Empty();

	if (SelectedActor)
	{
		SelectedActor->ReleaseOverlayState(EOverlayState::SelectUltraHand);
		SelectedActor->SetGrabState(false);
		SelectedActor = nullptr;

		PlayerVFXComponent->Deactivate();
	}
}

void UZCUltrahandSkillComponent::GrabActorAtHitPoint(const FHitResult& HitResult)
{
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->PlayerCameraManager->GetCameraViewPoint(CameraLocation, CameraRotation);

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (!HitComponent) return;

	float DistanceToImpact = FVector::Dist(CameraLocation, HitResult.ImpactPoint);
	InitialGrabDistance = FMath::Clamp(DistanceToImpact, MinGrabDistance, CatchMaxDistance);

	FTransform ComponentTransform = HitComponent->GetComponentTransform();
	GrabLocalOffset = ComponentTransform.InverseTransformPosition(HitResult.ImpactPoint);

	TargetWorldRotation = HitComponent->GetComponentQuat();
	SmoothedTargetWorldRotation = TargetWorldRotation;
	LastCameraYaw = CameraRotation.Yaw;

	GrabHandle->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, HitResult.ImpactPoint, TargetWorldRotation.Rotator());

	// VFX 및 상태 업데이트
	PlayerVFXComponent->SetAsset(PlayerToGrabbedVFX);
	PlayerVFXComponent->SetActive(true);
}

FVector UZCUltrahandSkillComponent::CalculateTargetGrabLocation() const
{
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->PlayerCameraManager->GetCameraViewPoint(CameraLocation, CameraRotation);

	const FVector CameraForward = CameraRotation.Vector();
	return CameraLocation + CameraForward * InitialGrabDistance;
}

void UZCUltrahandSkillComponent::UpdateActorOverlayStates(const TSet<AZCActor*>& NewActors, const TSet<AZCActor*>& OldActors)
{
	// 새로 감지된 액터 활성화
	for (AZCActor* Actor : NewActors)
	{
		if (Actor && !OldActors.Contains(Actor))
		{
			Actor->RequestOverlayState(EOverlayState::UltraHand);
		}
	}

	// 범위를 벗어난 액터 비활성화
	for (AZCActor* Actor : OldActors)
	{
		if (Actor && !NewActors.Contains(Actor))
		{
			Actor->ReleaseOverlayState(EOverlayState::UltraHand);
		}
	}
}

void UZCUltrahandSkillComponent::SpawnGluePoint()
{
	if (!GluePointActorClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	GluePoint = GetWorld()->SpawnActor<AZCGluePointActor>(GluePointActorClass, SpawnLocation, SpawnRotation, SpawnParams);

	SetGluePointVisible(false);
}

void UZCUltrahandSkillComponent::SetGluePointVisible(bool bVisible)
{
	if (!GluePoint) return;

	GluePoint->SetActorHiddenInGame(!bVisible);
}

void UZCUltrahandSkillComponent::UpdateGluePointTransform(const FVector& SourceLocation, const FVector& TargetLocation)
{
	if (!GluePoint) return;

	GluePoint->SetPosition(SourceLocation, TargetLocation);
}

void UZCUltrahandSkillComponent::SpawnGizmo()
{
	if (!GizmoClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	Gizmo = GetWorld()->SpawnActor<AZCGizmoActor>(GizmoClass, SpawnLocation, SpawnRotation, SpawnParams);

	SetGizmoVisible(false);
}

void UZCUltrahandSkillComponent::SetGizmoVisible(bool bVisible)
{
	if (!Gizmo) return;
	Gizmo->SetActorHiddenInGame(!bVisible);
}

void UZCUltrahandSkillComponent::UpdateGizmoTransform(const FVector& NewLocation)
{
	if (!Gizmo) return;
	if (PC && PC->PlayerCameraManager)
	{
		FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();

		FVector DirectionToCamera = CameraLocation - NewLocation;
		FRotator LookAtRotation = FRotationMatrix::MakeFromX(DirectionToCamera).Rotator();
		Gizmo->SetActorLocationAndRotation(NewLocation, LookAtRotation);
	}
}
