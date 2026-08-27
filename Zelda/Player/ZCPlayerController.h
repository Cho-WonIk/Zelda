// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameData/Enum/ZCItemType.h"
#include "ZCPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class AZCPlayerCharacter;
class AZCPlayerState;

class UZCPlayerDeviceComponent;
class UZCUltrahandSkillComponent;
class UZCHUDManagerComponent;
class UZCNiagaraComponent;

class UZCHUDLocalPlayerSubsystem;

enum class EFaceButtonEvent : uint32;

enum class ESkillType : uint8;

enum ECustomMovementMode : uint8;

UCLASS()
class ZELDA_API AZCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AZCPlayerController();

	// Override Function
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	virtual void InitPlayerState() override;
	virtual void ReceivedPlayer() override;

public:
	void SetPlayerVFX(UZCNiagaraComponent* NewVFX);
	UZCNiagaraComponent* GetPlayerVFX() { return ZCPlayerVFXComponent; }

	/*==========인풋 설정=========*/
public:
	void SetPlayerInputMappingContext(UInputMappingContext* NewContext, int32 Priority);
	void RemovePlayerInputMappingContext(UInputMappingContext* NewContext);

	// 인풋 액션
protected:
	// 상호작용 키
	void Interaction();

	// 오른손기능, HOLD
	// 짧게 누르면 가장 최근에 사용했던 기능으로 활성화 Canceled, LastRightHandSkiil()
	// 길게 누르면 선택화면 뜸 Trigger(매틱마다 발생), SelectRightHandSkill()
	// 이후 때면 선택한 기능 활성화 Completed, UseRightHandSkill()
	void LastRightHandSkill();
	void SelectRightHandSkill();
	void UseRightHandSkill();
	void ChoseRightHandSkill(const FInputActionValue& Value);
	
	void ShortCutScroll(const FInputActionValue& Value);
	void ShowShortCut(const FInputActionValue& Value);
	void HideShortCut(const FInputActionValue& Value);
	void HorseCall();

	UFUNCTION()
	void HandleInputMethod(ECommonInputType NewType);

protected:
	// 인게임 캐릭터 조작
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> InGameMappingContext;

	// 인게임 UI조작
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> InUIMappingContext;

	// 울트라 핸드 조작 모드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> UltrahandMappingContext;

	// 기어 탑승 모드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> RideMappingContext;

	/*===================================================================*/
	// NPC대화, 아이템습득, 클라이밍 다운
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteraactionAction;

	// 숏컷 스크롤 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShortCutScollAction;

	// 숏컷
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShortCutAction;

	// 말부르기 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> HorseCallAction;

	/*===================================================================*/
	// 오른손 기능
	// 오른손 기능 UI활성화
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RightHandAction;

	// 오른손 기능 마우스/R스틱으로 선택 기능
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ChoseRightHandSkillAction;

	/*===================================================================*/
	// 기어 조작 취소
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RideOffAction;

protected:
	// 시커 스톤 컨트롤 객체
	// 실제 스킬은 플레이어 캐릭터에서 구현
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Device, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UZCPlayerDeviceComponent> DeviceControlComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Device, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UZCUltrahandSkillComponent> UltrahandSkillComponent;

	// UPhysicsHandleComponent 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Device, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPhysicsHandleComponent> GrapComponent;

public:
	void ActiveSkill(ESkillType SkillType);
	void DeactiveSkill(ESkillType SkillType);

	/*==========플레이어===========*/
public:
	// 플레이어 스테이트
	AZCPlayerState* GetZCPlayerState() const { return ZCPlayerState; }

	// 인벤토리 기능
public:
	void NotifyItemInRange(AActor* ItemActor);
	void NotifyItemOutRange(AActor* ItemActor);

	void SpawnItemFromShortCut();

protected:
	// 플레이어와 가장 가까운 거리에 있는 아이템을 습득하는 로직
	void PickUpNearItem();

	// 기어에 승차
	void RideOnGear();
	void RideOffGear();

protected:
	UFUNCTION()
	void OnPlayerMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode);

private:
	TSet<TWeakObjectPtr<class AActor>> ItemInRangeSet;

private:
	AZCPlayerCharacter* ZCPlayerCharacter = nullptr;
	AZCPlayerState* ZCPlayerState = nullptr;
	UZCNiagaraComponent* ZCPlayerVFXComponent = nullptr;

	UZCHUDLocalPlayerSubsystem* HUDLocalPlayerSubsystem = nullptr;
};
