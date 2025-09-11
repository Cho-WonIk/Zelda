// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "Utils/ZCPathFollowUtils.h"
#include "ZCPathFollowingComponent.generated.h"

USTRUCT()
struct FZCNavMeshValidationSettings
{
	GENERATED_BODY()

	/** 내비게이션 지면(NavMesh) 위에 있는지 검사 기능 활성화 */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "NavMesh 검사 활성화"), Category = "NavMesh 유효성 검사")
	bool bNavmeshCheck = false;

	/** NavMesh 위에 있는지 확인하는 검사 주기(초 단위) */
	UPROPERTY(EditAnywhere, config, meta = (ClampMin = "0.1", EditCondition = bNavmeshCheck, DisplayName = "검사 주기 (초)"), Category = "NavMesh 유효성 검사")
	float NavmeshCheckInterval = 0.1f;

	/** NavMesh에 투영할 때 사용하는 전방 거리 */
	UPROPERTY(EditAnywhere, config, meta = (ClampMin = "0.0", EditCondition = bNavmeshCheck, DisplayName = "전방 검사 거리"), Category = "NavMesh 유효성 검사")
	float ForwardCheckDistance = 25.f;

	/** 유효성 검사 투영 범위 */
	UPROPERTY(EditAnywhere, config, meta = (ClampMin = "0.0", EditCondition = bNavmeshCheck, DisplayName = "투영 범위"), Category = "NavMesh 유효성 검사")
	float ForwardCheckExtent = 25.f;
};

USTRUCT()
struct FZCCharacterSmoothingData
{
	GENERATED_BODY()

	bool bUseControllerRotationYaw = false;
	bool bOrientRotationToMovement = false;
};

UCLASS()
class ZELDA_API UZCPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

public:
	UZCPathFollowingComponent();

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "부드러운 경로 활성화", Category = "AI|Navigation"))
	void TogglePathSmoothing(bool bEnable);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "부드러운 경로 활성화 여부", Category = "AI|Navigation"))
	FORCEINLINE bool IsPathSmoothingEnabled() const { return bActivePathSmoothing; }

protected:
	virtual void OnRegister() override;
	virtual void OnNewPawn(APawn* NewPawn) override;
	virtual void OnPathUpdated() override;
	virtual void FollowPathSegment(float DeltaTime) override;
	virtual bool HasReachedCurrentTarget(const FVector& CurrentLocation) const override;
	virtual void OnPathFinished(const FPathFollowingResult& Result) override;

private:
	// 언리얼 엔진 Config로부터 설정을 받아옴
	FZCPathSimplificationSettings SimplificationSettings;
	FZCPathSmoothingSettings SmoothingSettings;
	FZCNavMeshValidationSettings NavMeshValidationSettings;

	// 디버그 색상 설정
	FColor SmoothedPathDebugColor = FColor::Green;
	FColor DefaultPathDebugColor = FColor::Red;

	// 캐릭터 컨트롤 옵션도 같이 수정되야 하므로 블루프린트 노출 불가
	bool bActivePathSmoothing = true;

	TArray<FNavPathPoint> DefaultPathPoints;

	FTimerHandle NavMeshCheckTimerHandle;

	TObjectPtr<class AAIController> AIController;
	TObjectPtr<class ACharacter> ControllerCharacter;

	FZCCharacterSmoothingData CharacterSmoothingData;

private:
	void ToggleSmoothRotation(bool bEnable);
	void ValidateNavMeshLocation();
};
