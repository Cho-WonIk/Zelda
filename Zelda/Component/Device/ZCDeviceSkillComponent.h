// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/ZCDeviceInterface.h"
#include "ZCDeviceSkillComponent.generated.h"

enum class ESkillType : uint8;

class UEnhancedInputComponent;
class UInputAction;
class AZCPlayerController;
class AZCActor;
class UZCNiagaraComponent;
class UZCHUDLocalPlayerSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillDelegate, ESkillType);

UCLASS( ClassGroup=(Custom), abstract, BlueprintType)
class ZELDA_API UZCDeviceSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCDeviceSkillComponent();

public:
	FOnSkillDelegate OnDeactivate;
public:	
	virtual void SetupInputComponent(UEnhancedInputComponent* InputComponent);

	void SetPlayerController(AZCPlayerController* NewPC) { PC = NewPC; }
	void SetPlayerVFXComponent(UZCNiagaraComponent* NewVFX) { PlayerVFXComponent  = NewVFX; }
	virtual void SetHUDSubsystem(UZCHUDLocalPlayerSubsystem* NewLocalSub) { HUDSubsystem = NewLocalSub; }

public:
	virtual void Activate(bool bReset = false) override;;
	virtual void Deactivate() override;
	virtual bool ShouldActivate() const override;

protected:
	virtual void Action();
	virtual void Cancel();

protected:
	/**
	* 카메라 위치로부터 캐릭터를 향하는 방향 벡터를 기반으로,
	* 캐릭터 위치를 시작점으로 하는 레이(Line)의 Start/End를 계산한다.
	 *
	* @param OutStart	레이의 시작점 (캐릭터 중심 위치)
	* @param OutEnd	캐릭터 위치에서 (카메라 → 캐릭터) 방향으로 Distance만큼 이동한 지점
	* @param Distance	캐릭터 위치로부터 해당 방향으로 뻗을 거리
	*
	* @return 캐릭터 또는 카메라 정보를 획득하지 못할 경우 false, 성공 시 true
	*/
	bool GetCameraRayToCharacterRay(FVector& OutStart, FVector& OutEnd, float Distance) const;

	/**
	* 지정된 위치(Pos)와 회전(Rot)을 기준으로,
	* 주어진 충돌 형태(Shape)에 대해 Overlap 검사를 수행한다.
	*
	* @param OutOverlapResults	Overlap된 모든 액터 및 컴포넌트 결과가 저장되는 배열
	* @param Pos				Overlap 검사를 수행할 월드 위치
	* @param Rot				Overlap 검사를 수행할 월드 회전(쿼터니언)
	* @param Shape				검사에 사용할 충돌 형태(구, 캡슐, 박스)
	*
	* @return Overlap			검사 실패 시 false, 하나 이상 검사 성공 시 true
	*/
	bool DeviceOverlapMultiByChannel(TArray<FOverlapResult>& OutOverlapResults, const FVector &Pos, const FQuat &Rot, const FCollisionShape &Shape) const;


	/**
	* 지정된 시작(Start) 위치에서 종료(End) 위치까지,
	* 주어진 충돌 형태(Shape)을 회전(Rot) 상태로 스윕하여
	* 가장 먼저 충돌한 Hit 정보를 반환한다.
	 *
	* @param OutHit   스윕 과정에서 처음으로 맞은 액터와 충돌 지점 등의 정보가 저장됨
	 * @param Start    스윕 시작 지점 (월드 좌표)
	* @param End      스윕 종료 지점 (월드 좌표)
	 * @param Rot      스윕에 사용될 Shape의 회전(쿼터니언)
	* @param Shape    스윕에 사용할 충돌 형태 (구/캡슐/박스)
	*
	* @return true    충돌한 액터가 있어 OutHit가 유효할 때
	* @return false   스윕 경로 중 어떤 액터와도 충돌하지 않았을 때
	*/
	bool DeviceSweepSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& End, const FQuat& Rot, const FCollisionShape& Shape) const;

	/** 플레이어와 가장 가까운 AZCActor를 반환하는 함수, TArray, TSet버전*/
	AZCActor* GetNearestActorFromPlayer(const TArray<AZCActor*>& Actors) const;
	AZCActor* GetNearestActorFromPlayer(const TSet<AZCActor*>& Actors) const;

	/** 특정 지점과 가장 가까운 액터를 반환하는 함수, TArray, TSet버전*/
	AZCActor* GetNearestActorAtLocation(const FVector& Location, const TArray<AZCActor*>& Actors) const;
	AZCActor* GetNearestActorAtLocation(const FVector& Location, const TSet<AZCActor*>& Actors) const;

private:
	template<typename ContainerType>
	AZCActor* GetNearestActorFromPlayer_Internal(const ContainerType& Actors) const;

	template<typename ContainerType>
	AZCActor* FindNearestActorAtLocation_Internal(const FVector& Location, const ContainerType& Actors) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Type, meta = (AllowPrivateAccess = "true"))
	ESkillType SkillType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SelectAction;

	// 오른손 기능 마우스/R스틱으로 선택 기능
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CancelAction;

	ECollisionChannel DeviceSkillCollisionChannel = ECC_Visibility;

protected:
	class AZCPlayerController* PC = nullptr;
	class UZCNiagaraComponent* PlayerVFXComponent = nullptr;
	class UZCHUDLocalPlayerSubsystem* HUDSubsystem = nullptr;
};
