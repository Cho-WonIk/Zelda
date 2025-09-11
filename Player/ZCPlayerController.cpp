// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCPlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "UI/HUD/ZCHUDCommonWidget.h"
#include "UI/Common/FaceButton/ZCFaceButtonWidget.h"
#include "Character/Player/ZCPlayerCharacter.h"
#include "Component/Climb/ZCCharacterMovementComponent.h"
#include "Player/ZCPlayerState.h"
#include "Component/Inventory/ZCInventoryComponent.h"
#include "Interface/ZCItemInterface.h"
 

AZCPlayerController::AZCPlayerController()
{
}

void AZCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ZCHUDWidget = CreateWidget<UZCHUDCommonWidget>(this, ZCHUDClass);
	if (ZCHUDWidget)
	{
		ZCHUDWidget->AddToViewport(0);
	}

	SetPlayerInputMappingContext(InGameMappingContext, 1);
	SetPlayerInputMappingContext(InGameUIMappingContext, 0);

}

void AZCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ZCPlayerCharacter = Cast<AZCPlayerCharacter>(InPawn);
	if (ZCPlayerCharacter)
	{
		if (UZCCharacterMovementComponent* MovementComp = ZCPlayerCharacter->GetZCCharacterMovementComponent())
		{
			MovementComp->OnMovementChangeDelegate.AddDynamic(this, &AZCPlayerController::OnMovementChange);
		}
	}
}

void AZCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(DPadUpAction, ETriggerEvent::Started, this, &AZCPlayerController::PressDPadUp);
		EnhancedInputComponent->BindAction(DPadDownAction, ETriggerEvent::Started, this, &AZCPlayerController::PressDPadDown);
		EnhancedInputComponent->BindAction(DPadLeftAction, ETriggerEvent::Started, this, &AZCPlayerController::PressDPadLeft);
		EnhancedInputComponent->BindAction(DPadRightAction, ETriggerEvent::Started, this, &AZCPlayerController::PressDPadRight);

		EnhancedInputComponent->BindAction(DPadUpAction, ETriggerEvent::Completed, this, &AZCPlayerController::ReleaseDPadUp);
		EnhancedInputComponent->BindAction(DPadLeftAction, ETriggerEvent::Completed, this, &AZCPlayerController::ReleaseDPadLeft);
		EnhancedInputComponent->BindAction(DPadRightAction, ETriggerEvent::Completed, this, &AZCPlayerController::ReleaseDPadRight);

		EnhancedInputComponent->BindAction(ShortCutScollAction, ETriggerEvent::Triggered, this, &AZCPlayerController::ShortCutScroll);
	}
}

void AZCPlayerController::SetPlayerInputMappingContext(UInputMappingContext* NewContext, int32 Priority)
{
	if (!NewContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(NewContext, Priority);
	}
}

void AZCPlayerController::FaceButtonRight()
{
	switch (CurrentDisplayInteractionEvent)
	{
	case PickUp:
		PickUpNearItem();
		break;

	case ClimbDown:
		break;

	case Interact:
		break;

	case Talk:
		break;

	default:
		break;
	}
}

void AZCPlayerController::ShortCutScroll(const FInputActionValue& Value)
{
	if (!IsPaused()) return;

	const float ScrollValue = Value.Get<float>();

	if (FMath::IsNearlyZero(ScrollValue)) return;

	const int32 Direction = ScrollValue > 0 ? 1 : -1;

	ZCHUDWidget->ScrollShortCutItemList(Direction);
}

void AZCPlayerController::PressDPadUp()
{
	SetPlayerInputMappingContext(InGameUIMappingContext, 2);

	CurrentShortCutItemType = EItemType::Normal;

	ZCHUDWidget->ShowShortCutUI(EItemType::Normal);

	SetPause(true);
}

void AZCPlayerController::PressDPadDown()
{
	// 말부르기
}

void AZCPlayerController::PressDPadLeft()
{
	SetPlayerInputMappingContext(InGameUIMappingContext, 2);

	CurrentShortCutItemType = EItemType::Shield;

	ZCHUDWidget->ShowShortCutUI(EItemType::Shield);

	SetPause(true);
}

void AZCPlayerController::PressDPadRight()
{
	SetPlayerInputMappingContext(InGameUIMappingContext, 2);

	CurrentShortCutItemType = EItemType::Weapon;

	ZCHUDWidget->ShowShortCutUI(EItemType::Weapon);

	SetPause(true);
}

void AZCPlayerController::GoToInGameMappingContext()
{
	SetPlayerInputMappingContext(InGameMappingContext, 0);
	ZCHUDWidget->HideShortCutUI();
	SetPause(false);
}

void AZCPlayerController::ReleaseDPadUp()
{
	SpawnItem();
	GoToInGameMappingContext();
}

void AZCPlayerController::ReleaseDPadLeft()
{
	SpawnItem();
	GoToInGameMappingContext();
}

void AZCPlayerController::ReleaseDPadRight()
{
	SpawnItem();
	GoToInGameMappingContext();
}

void AZCPlayerController::NotifyItemInRange(AActor* ItemActor)
{
	ItemInRangeSet.Add(ItemActor);

	AddInteractionEvent(EFaceButtonEvent::PickUp);
}

void AZCPlayerController::NotifyItemOutRange(AActor* ItemActor)
{
	ItemInRangeSet.Remove(ItemActor);
	if (ItemInRangeSet.IsEmpty())
	{
		RemoveInteractionEvent(EFaceButtonEvent::PickUp);
	}
}

void AZCPlayerController::SpawnItem()
{
	int32 CurrentIndex = ZCHUDWidget->GetCurrentShortCutItemID();
	if (CurrentIndex == -1)
	{
		switch (CurrentShortCutItemType)
		{
		case EItemType::Normal:
			break;
		case EItemType::Weapon:
			ZCPlayerCharacter->SetNewWeapon(nullptr);
			break;
		case EItemType::Shield:
			ZCPlayerCharacter->SetNewShield(nullptr);
			break;
		default:
			break;
		}
	}
	else
	{
		// 인벤토리에서 아이템 가져오기
		AZCPlayerState& ZCPlayerState = *GetZCPlayerStateChecked<AZCPlayerState>();

		switch (CurrentShortCutItemType)
		{
		case EItemType::Normal:
			break;

		case EItemType::Weapon:
			// 인벤토리에서 아이템 가져오기
			ZCPlayerCharacter->SetNewWeapon(ZCPlayerState.GetInventoryComponent()->SpawnWeaponItem(CurrentIndex, FTransform::Identity));
			break;

		case EItemType::Shield:
			// 인벤토리에서 아이템 가져오기
			ZCPlayerCharacter->SetNewShield(ZCPlayerState.GetInventoryComponent()->SpawnShieldItem(CurrentIndex, FTransform::Identity));
			break;
		default:
			break;
		}
	}
}

void AZCPlayerController::PickUpNearItem()
{
	if (ItemInRangeSet.IsEmpty()) return;

	FVector PlayerLocation = GetPawn()->GetActorLocation();

	AActor* ClosestItem = nullptr;
	float MinXYDistance = FLT_MAX;
	float MinZDistance = FLT_MAX;

	// 안전한 순회 및 유효하지 않은 아이템 제거
	for (auto It = ItemInRangeSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		AActor* ItemActor = It->Get();
		FVector ItemLocation = ItemActor->GetActorLocation();

		float XYDistance = FVector::Dist2D(PlayerLocation, ItemLocation);
		float ZDistance = FMath::Abs(PlayerLocation.Z - ItemLocation.Z);

		if (XYDistance < MinXYDistance)
		{
			MinXYDistance = XYDistance;
			MinZDistance = ZDistance;
			ClosestItem = ItemActor;
		}
		else if (FMath::IsNearlyEqual(XYDistance, MinXYDistance))
		{
			if (ZDistance < MinZDistance)
			{
				MinZDistance = ZDistance;
				ClosestItem = ItemActor;
			}
		}
	}

	if (ClosestItem)
	{
		// 정확히 일치하는 WeakPtr 찾아 제거
		for (auto It = ItemInRangeSet.CreateIterator(); It; ++It)
		{
			if (It->IsValid() && It->Get() == ClosestItem)
			{
				It.RemoveCurrent();
				break;
			}
		}

		if (IZCItemInterface* AddItem = Cast<IZCItemInterface>(ClosestItem))
		{
			// 아이템 습득 로직
			if (TUniquePtr<FZCItemTable> Item = AddItem->OnItemPickUp())
			{
				GetZCPlayerStateChecked<AZCPlayerState>()->GetInventoryComponent()->AddItem(TUniquePtr<FZCItemTable>(static_cast<FZCItemTable*>(Item.Release())));
			}

			// NotifyItemOutRange를 호출 안해도 Item의 Destory가 호출될때 자동으로 호출됨
		}
	}
}

void AZCPlayerController::UpdateHUDInteractionEvent()
{
	if (!ZCHUDWidget) return;

	switch (CurrentDisplayInteractionEvent)
	{
	case None:
		ZCHUDWidget->HideFaceButtonUI();
		break;
	case PickUp:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("습득")));
		break;
	case ClimbDown:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("내려가기")));
		break;
	case Interact:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("상호작용")));
		break;
	case Talk:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("대화")));
		break;
	case Ambush:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::YButton, FText::FromString(TEXT("습격")));
		break;
	default:
		ZCHUDWidget->HideFaceButtonUI();
		break;
	}
}

void AZCPlayerController::SelectTopInteractionEvent()
{
	if (InteractionEventHeap.Num() > 0)
	{
		CurrentDisplayInteractionEvent = InteractionEventHeap[0];

		UpdateHUDInteractionEvent();
	}
	else
	{
		ClearInteractionEvent();
	}
}

void AZCPlayerController::AddInteractionEvent(const EFaceButtonEvent NewEvent)
{
	if (NewEvent == EFaceButtonEvent::None || NewEvent == EFaceButtonEvent::Max) return;

	if (!InteractionEventHeap.Contains(NewEvent))
	{
		InteractionEventHeap.HeapPush(NewEvent, FaceButtonCompare);
		SelectTopInteractionEvent();
	}
}

void AZCPlayerController::RemoveInteractionEvent(const EFaceButtonEvent RemoveEvent)
{
	InteractionEventHeap.RemoveSingle(RemoveEvent);
	InteractionEventHeap.Heapify(FaceButtonCompare);
	SelectTopInteractionEvent();
}

void AZCPlayerController::ClearInteractionEvent()
{
	CurrentDisplayInteractionEvent = EFaceButtonEvent::None;
	InteractionEventHeap.Empty();
	UpdateHUDInteractionEvent();
}

void AZCPlayerController::OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode)
{
	if (NewMovementMode != EMovementMode::MOVE_Custom)
	{
		switch (NewMovementMode)
		{
		case EMovementMode::MOVE_Falling:
			ZCHUDWidget->ShowActionIndicatorUI(Widget::ActionIndicatorIdx::Falling);
			break;
		default:
			ZCHUDWidget->HideActionIndicatorUI();
			break;
		}
	}
	else
	{
		switch (NewCustomMovementMode)
		{
		case CMOVE_Climbing:
			ZCHUDWidget->ShowActionIndicatorUI(Widget::ActionIndicatorIdx::Climb);
			break;

		case CMOVE_Gliding:
			ZCHUDWidget->ShowActionIndicatorUI(Widget::ActionIndicatorIdx::Glide);
			break;

		case CMOVE_MAX:
			ZCHUDWidget->HideActionIndicatorUI();
			break;

		default:
			break;
		}
	}

}
