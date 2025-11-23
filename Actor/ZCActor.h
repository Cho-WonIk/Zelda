// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Physics/ZCCollision.h"
#include "Physics/ZCShape.h"
#include "GameData/Table/ZCActorTable.h"
#include "ZCActor.generated.h"

class UZCNiagaraComponent;
class UZCMaterialStateComponent;
class UZCPhysActorComponent;

class UZCUltrahandSkillComponent;

class UMaterialInstanceDynamic;

class IZCUltrahandDeviceInterface;

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EZCActorState : uint8
{
	// 어떠한 것도 불가능
	None				= 0 UMETA(DisplayName = "None"),

	Grab				= 1 << 1 UMETA(DisplayName = "잡은상태"),
};
ENUM_CLASS_FLAGS(EZCActorState);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor))
enum class EOverlayState : uint8
{
	None			= 0,        // 없음
	ItemHighlight	= 1 << 0,   // 미습득 아이템
	UltraHand		= 1 << 1,   // 울트라 핸드 상호작용 가능
	SelectUltraHand = 1 << 2,
};

ENUM_CLASS_FLAGS(EOverlayState);

UCLASS()
class ZELDA_API AZCActor : public AActor
{
	GENERATED_BODY() 
	
public:	
	// Sets default values for this actor's properties
	AZCActor(const FObjectInitializer& ObjectInitializer);

public:
	FORCEINLINE UZCMaterialStateComponent* GetMaterialStateComponent() const { return MaterialStateComponent; }
	FORCEINLINE UZCNiagaraComponent* GetZCNiagaraComponent() const { return NiagaraComponent; }

	virtual void PostInitializeComponents() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	/*===========생성 함수===========*/
	virtual void Initialize(FZCActorTable* NewInfo);

	UMeshComponent* GetMesh() const { return Mesh; }

	/*=======오버레이 머티리얼 관련 함수=========*/
public:
	void RequestOverlayState(EOverlayState StateToRequest);
	void ReleaseOverlayState(EOverlayState StateToRelease);

	/*======= 플레이어 스킬 관련 함수 =======*/
	void SetGrabState(bool bGrab, IZCUltrahandDeviceInterface* NewOwner = nullptr);

	/*======== 콜리전 관련 함수들 =============*/
protected:
	UFUNCTION()
	virtual void OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void UpdateOverlayMaterial();
	void SetOutlineMaterial(UMaterialInterface* NewMaterial);

protected:
	static FName MeshComponentName;
	static FName InteractionAreaName;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<class UZCNiagaraComponent> NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<class UZCMaterialStateComponent> MaterialStateComponent;

	TObjectPtr<class UMeshComponent> Mesh;

	TObjectPtr<class UShapeComponent> InteractionArea;

protected:
	// 액터가 원소와 상호작용하는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zelda|Actor", meta = (AllowPrivateAccess = "true", DisplayName = "원소 상호작용 여부"))
	bool bCanElementalReaction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zelda|Actor")
	struct FZCShape Shape;

	struct FZCActorTable* Info = nullptr;

protected:
	EZCActorState ZCActorState = EZCActorState::None;

	TObjectPtr<class UMaterialInterface> CurrentOverlayMaterial;

	/** 아이템 오버레이용 동적 머티리얼*/
	TObjectPtr<class UMaterialInstanceDynamic> ItemOverlayMID;

	/** 울트라 핸드용 동적 머티리얼 */
	TObjectPtr<class UMaterialInstanceDynamic> UltraHandMID;

	EOverlayState CurrentOverlayState = EOverlayState::None;

	class IZCUltrahandDeviceInterface* UltraHandInterface = nullptr;
};
