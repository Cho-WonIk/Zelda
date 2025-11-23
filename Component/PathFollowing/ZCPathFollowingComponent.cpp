// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PathFollowing/ZCPathFollowingComponent.h"
#include "AIController.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Settings/PathFollow/ZCPathFollowWorldSettings.h"

#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::Navigation
{
	static int32 DrawDebugPathCount = 0;
	static FAutoConsoleVariableRef CVarDrawNavPath(
		Zelda::Debug::Navigation::path,
		DrawDebugPathCount,
		TEXT("네비게이션 경로 시각화\n"
			"0: 비활성화\n"
			"1: 현재 경로만 시각화\n"
			"2: 기본 경로와 캣멀 롬 경로 시각화"),
		ECVF_Default
	);
}
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCPathFollowingComponent)

UZCPathFollowingComponent::UZCPathFollowingComponent()
{
}

void UZCPathFollowingComponent::TogglePathSmoothing(bool bEnable)
{
	if (bActivePathSmoothing != bEnable)
	{
		bActivePathSmoothing = bEnable;
		ToggleSmoothRotation(bEnable);
	}
}

void UZCPathFollowingComponent::OnRegister()
{
	Super::OnRegister();

	if (const UZCPathFollowWorldSettings* DefaultSettings = GetDefault<UZCPathFollowWorldSettings>())
	{
		SimplificationSettings = DefaultSettings->SimplificationSettings;
		SmoothingSettings = DefaultSettings->SmoothingSettings;
		NavMeshValidationSettings = DefaultSettings->NavMeshValidationSettings;
		DefaultPathDebugColor = DefaultSettings->DefaultPathDebugColor;
		SmoothedPathDebugColor = DefaultSettings->SmoothedPathDebugColor;
	}
}

void UZCPathFollowingComponent::OnNewPawn(APawn* NewPawn)
{
	Super::OnNewPawn(NewPawn);
	
	// 설정 초기화
	CharacterSmoothingData = FZCCharacterSmoothingData();

	ControllerCharacter = Cast<ACharacter>(NewPawn);
	if (ControllerCharacter && ControllerCharacter->GetCharacterMovement())
	{
		CharacterSmoothingData.bUseControllerRotationYaw = ControllerCharacter->bUseControllerRotationYaw;
		CharacterSmoothingData.bOrientRotationToMovement = ControllerCharacter->GetCharacterMovement()->bOrientRotationToMovement;

		ToggleSmoothRotation(true);
	}
}

void UZCPathFollowingComponent::OnPathUpdated()
{
	Super::OnPathUpdated();

	if (Path.IsValid() && Path->IsValid())
	{
		// 기존 경로 저장
		DefaultPathPoints = Path->GetPathPoints();

		// 경로점이 3개 이상일때 적용
		if (bActivePathSmoothing && Path->GetPathPoints().Num() > 2)
		{
			// 경로 부드럽게 하기
			ZCPathSmoothing::SmoothNavPath(Path->GetPathPoints(), SimplificationSettings, SmoothingSettings);

			// 주기적으로 NavMesh 위치를 검증
			if (NavMeshValidationSettings.bNavmeshCheck)
			{
				GetWorld()->GetTimerManager().SetTimer(NavMeshCheckTimerHandle, this, &UZCPathFollowingComponent::ValidateNavMeshLocation, NavMeshValidationSettings.NavmeshCheckInterval, true);
			}
		}
	}
}

void UZCPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	Super::FollowPathSegment(DeltaTime);

#if !UE_BUILD_SHIPPING
	const int32 DrawNavPathMode = Zelda::Debug::Navigation::DrawDebugPathCount;
	if (DrawNavPathMode >= 1 && Path.IsValid() && Path->IsValid())
	{
		ZCPathSmoothing::DrawPath(GetWorld(), Path->GetPathPoints(), bActivePathSmoothing ? SmoothedPathDebugColor : DefaultPathDebugColor, 5.f);
	}

	if (DrawNavPathMode == 2 && bActivePathSmoothing)
	{
		ZCPathSmoothing::DrawPath(GetWorld(), DefaultPathPoints, DefaultPathDebugColor, 5.f);

		const FString Label = bActivePathSmoothing ? TEXT("부드러운 경로로 이동 중") : TEXT("기본 경로로 이동 중");

		DrawDebugString(GetWorld(), ControllerCharacter->GetActorLocation() + FVector(0, 0, 80.f), Label, nullptr, FColor::Cyan, 0.0f, true);
	}
#endif
}

bool UZCPathFollowingComponent::HasReachedCurrentTarget(const FVector& CurrentLocation) const
{
	FVector currentLocationUpdate = CurrentLocation;

	const FVector CurrentTarget = GetCurrentTargetLocation();
	if (bActivePathSmoothing)
	{
		// 평면 거리만 비교
		currentLocationUpdate.Z = CurrentTarget.Z;
	}

	return Super::HasReachedCurrentTarget(currentLocationUpdate);
}

void UZCPathFollowingComponent::OnPathFinished(const FPathFollowingResult& Result)
{
	Super::OnPathFinished(Result);
	
	DefaultPathPoints.Empty();
	GetWorld()->GetTimerManager().ClearTimer(NavMeshCheckTimerHandle);
}

void UZCPathFollowingComponent::ToggleSmoothRotation(bool bEnable)
{
	if (SmoothingSettings.bSmoothCharacterRotation && ControllerCharacter && ControllerCharacter->GetCharacterMovement())
	{
		ControllerCharacter->bUseControllerRotationYaw = bEnable ? false : CharacterSmoothingData.bUseControllerRotationYaw;
		ControllerCharacter->GetCharacterMovement()->bOrientRotationToMovement = bEnable ? true : CharacterSmoothingData.bOrientRotationToMovement;
	}
}

void UZCPathFollowingComponent::ValidateNavMeshLocation()
{
	if (!ControllerCharacter || !Path.IsValid() || !Path->IsValid()) return;
	
	// 전방 위치 계산
	FNavLocation NavLocation;
	const FVector ProjectionExtent = FVector(NavMeshValidationSettings.ForwardCheckExtent, NavMeshValidationSettings.ForwardCheckExtent, NavMeshValidationSettings.ForwardCheckExtent * 2.f);
	const FVector NewNavLocation = GetCurrentNavLocation();
	const FVector ForwardLocation = ControllerCharacter->GetActorForwardVector() * NavMeshValidationSettings.ForwardCheckDistance + NewNavLocation;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys && !NavSys->ProjectPointToNavigation(ForwardLocation, NavLocation, ProjectionExtent))
	{
		// 전방에 NavMesh가 없을 경우 재탐색
		const FNavPathPoint CurrentGoal = Path->GetPathPoints().Last();

		const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
		const FNavAgentProperties& AgentProperties = ControllerCharacter->GetNavAgentPropertiesRef();
		FPathFindingQuery Query(GetOwner(), *NavData, NewNavLocation, CurrentGoal);
		Query.SetNavAgentProperties(AgentProperties);
		Query.SetPathInstanceToUpdate(Path);

		const FPathFindingResult PathResult = NavSys->FindPathSync(Query);
		if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
		{
			HandlePathUpdateEvent();
		}
	}
}
