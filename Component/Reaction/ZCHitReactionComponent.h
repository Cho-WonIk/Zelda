// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsControlComponent.h"
#include "Struct/ZCReactionEnum.h"
#include "ZCHitReactionComponent.generated.h"

class UMotionWarpingComponent;
class UPhysicsControlComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCHitReactionComponent();

protected:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Init(UMotionWarpingComponent& InMotionWarpingComponent, UPhysicsControlComponent& InPhysicsControlComponent, USkeletalMeshComponent& InMesh);

protected:
	void UpdatePhysics(float DeltaTime);
	void UpdateMontionWarping();

public:
	void PerformHitReaction(const AActor* HitCauser, const FHitResult& HitResult, EHitStrength InputHitStrength, bool UseCauseDeath, bool UseHitCurserDirection);

protected:
	UAnimMontage* StartHitReaction(const AActor* HitCauser, const FHitResult& HitResult, EHitStrength InputHitStrength, bool UseCauseDeath, bool UseHitCurserDirection);

	void PerformMontage(UAnimMontage* Montage);
	void PerformPhysicsReaction();

	UFUNCTION()
	void NotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION()
	void NotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	// 애니메이션과 캐릭터의 차이로 인한 오차를 조정할 때 사용 이후 StopPlayNewMontage 호출시 원상 복귀, EHitStrenght::Heavy와 Explostion일 때 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction", meta = (DisplayName = "강한공격 피격 시 Mesh위치 조정"))
	TMap<EHitStrength, FVector> MeshLocationOffset;

	FVector OriginalMeshLocation;

protected:
	TObjectPtr<class USkeletalMeshComponent> MeshComponent;

	FName HitBoneName;
	FVector HitDir = FVector::ZeroVector;
	bool bUseHitCurserDirection = false;
	bool bCanPlayNewMontage = true;

	// 물리 액션 변수
	bool StartPhysicsReaction = false;
	float CurrentPhysicsReactionDuration = 1.5f;

	// 캐릭터의 죽음 모션을 재생할지 여부, Chooser 테이블이 참조함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly = "Hit Reaction", meta = (DisplayName = "캐릭터가 죽음"))
	bool bCauseDeath = false;

	// 캐릭터가 죽음 모션을 재생 중인지 여부
	bool bIsDying = false;

	// 캐릭터가 죽은 상태인지 여부(캐릭터 스탯과 별도의 애니메이션 상태)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly = "Hit Reaction", meta = (DisplayName = "캐릭터가 죽음 상태"))
	bool bIsDeath = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (DisplayName = "중심 뼈대"))
	FName CenterOfMassBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (DisplayName = "모션 워핑 왜곡 거리"))
	TMap<EHitStrength, float> MotionWarpingDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (DisplayName = "부위별 맵핑"))
	TMap<FName, EHitPosition> HitPosMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Physics", meta = (DisplayName = "물리 반응 지속 시간"))
	float PhysicsReactionDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Physics", meta = (DisplayName = "Physics Control 데이터"))
	struct FPhysicsControlData PhysicsControlData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Physics", meta = (DisplayName = "Body Modifier 데이터"))
	struct FPhysicsControlModifierData BodyModifierData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Physics", meta = (DisplayName = "Physics 제외 부위"))
	TArray<EHitPosition> DisablePhysicsParts = { EHitPosition::Pelvis, EHitPosition::Other, EHitPosition::LeftCalf, EHitPosition::RightCalf };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Physics", meta = (DisplayName = "Physics 강도"))
	TMap<EHitStrength, float> PhysicsReationStrength;


protected:
	// https://forums.unrealengine.com/t/use-chooser-plugin-in-c/2327532/3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction Chooser Table")
	TObjectPtr<class UChooserTable> HitReactionChooserTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Reaction Chooser Table", meta = (DisplayName = "스켈레톤 애셋"))
	TObjectPtr<class USkeletalMesh> MeshAsset;

	// Chooser 테이블이 참조함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Reaction", meta = (DisplayName = "맞은 부위"))
	EHitPosition HitPosition = EHitPosition::Head;

	// Chooser 테이블이 참조함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Reaction", meta = (DisplayName = "맞은 방향"))
	EZCHitDirection HitDirection = EZCHitDirection::Front;

	// Chooser 테이블이 참조함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Reaction", meta = (DisplayName = "맞은 강도"))
	EHitStrength HitStrength = EHitStrength::Light;


private:
	void InitPhysicsControl();

	EZCHitDirection GetHitDirection(const FHitResult& HitResult, const AActor* HitCauser) const;
	EHitPosition GetHitPosition(const FName& BoneName) const;

private:
	class UMotionWarpingComponent* MotionWarpingComponent;
	class UPhysicsControlComponent* PhysicsControlComponent;

	class UAnimInstance* ActorAnimInstance = nullptr;

	FTimerHandle PhysicsReactionTimerHandle;
};
