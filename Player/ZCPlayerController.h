// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Struct/Enum/ZCItemType.h"
#include "ZCPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class AZCPlayerCharacter;

class UZCHUDCommonWidget;
enum ECustomMovementMode : uint8;

UENUM(BlueprintType)
enum EFaceButtonEvent : uint8
{
	None		UMETA(DisplayName = "없음"),
	PickUp		UMETA(DisplayName = "습득"),
	ClimbDown	UMETA(DisplayName = "내려가기"),
	Interact	UMETA(DisplayName = "상호작용"),
	Talk		UMETA(DisplayName = "대화"),
	Ambush		UMETA(DisplayName = "습격"),
	Max			UMETA(Hidden),
};

UCLASS()
class ZELDA_API AZCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AZCPlayerController();

	// Override Function
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;


	/*==========인풋 설정=========*/
public:
	void SetPlayerInputMappingContext(UInputMappingContext* NewContext, int32 Priority);

	// 인풋 액션
public:
	// B버튼
	void FaceButtonRight();		// 플레이어 캐릭터에서 트리거 됨

protected:
	void ShortCutScroll(const FInputActionValue& Value);

	/*==============D패드 버튼(숏컷)=============*/
	void PressDPadUp();
	void PressDPadDown();
	void PressDPadLeft();
	void PressDPadRight();

	void GoToInGameMappingContext();

	void ReleaseDPadUp();
	void ReleaseDPadLeft();
	void ReleaseDPadRight();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> InGameMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> InGameUIMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShortCutScollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DPadUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DPadDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DPadLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DPadRightAction;


	/*==========플레이어===========*/
public:
	// 플레이어 스테이트
	template<typename PlayerStateT = APlayerState>
	PlayerStateT* GetZCPlayerState() const { return Cast<PlayerStateT>(PlayerState); }

	template<typename PlayerStateT = APlayerState>
	PlayerStateT* GetZCPlayerStateChecked() const
	{
		PlayerStateT* PS = GetZCPlayerState<PlayerStateT>();
		check(PS);
		return PS;
	}

	// 인벤토리 기능
public:
	void NotifyItemInRange(AActor* ItemActor);
	void NotifyItemOutRange(AActor* ItemActor);

	void SpawnItem();

protected:
	// 플레이어와 가장 가까운 거리에 있는 아이템을 습득하는 로직
	void PickUpNearItem();

private:
	TSet<TWeakObjectPtr<class AActor>> ItemInRangeSet;


	/*===========HUD===============*/
public:
	UFUNCTION(BlueprintCallable, Category = HUD)
	void UpdateHUDInteractionEvent();
	UFUNCTION(BlueprintCallable, Category = HUD)
	void SelectTopInteractionEvent();
	UFUNCTION(BlueprintCallable, Category = HUD)
	void AddInteractionEvent(const EFaceButtonEvent NewEvent);
	UFUNCTION(BlueprintCallable, Category = HUD)
	void RemoveInteractionEvent(const EFaceButtonEvent RemoveEvent);

	UFUNCTION(BlueprintCallable, Category = HUD)
	void ClearInteractionEvent();

protected:
	UFUNCTION()
	void OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UZCHUDCommonWidget> ZCHUDClass;

	TObjectPtr<UZCHUDCommonWidget> ZCHUDWidget;

	EItemType CurrentShortCutItemType = EItemType::None;

private:
	/*상호작용 이벤트 우선순위 큐(힙으로 구현)*/
	TArray<EFaceButtonEvent> InteractionEventHeap;

	EFaceButtonEvent CurrentDisplayInteractionEvent = EFaceButtonEvent::None;

	static int32 GetInteractionPriority(EFaceButtonEvent Event)
	{
		switch (Event)
		{
		case EFaceButtonEvent::None:		return 0;
		case EFaceButtonEvent::PickUp:		return 1;	// 아이템 습득
		case EFaceButtonEvent::ClimbDown:	return 2;	// 내려가기
		case EFaceButtonEvent::Interact:	return 3;	// 상호작용
		case EFaceButtonEvent::Talk:		return 4;	// 대화
		case EFaceButtonEvent::Ambush:		return 5;	// 습격
		default:							return 0;
		}
	}
	static bool FaceButtonCompare(const EFaceButtonEvent& A, const EFaceButtonEvent& B)
	{
		return GetInteractionPriority(A) > GetInteractionPriority(B);
	}

private:
	AZCPlayerCharacter* ZCPlayerCharacter = nullptr;
};
