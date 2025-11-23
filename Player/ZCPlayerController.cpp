// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCPlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "CommonInputSubsystem.h"

#include "UI/HUD/ZCHUDCommonWidget.h"
#include "UI/Common/FaceButton/ZCFaceButtonWidget.h"
#include "Character/Player/ZCPlayerCharacter.h"
#include "Player/ZCPlayerState.h"

#include "Interface/ZCItemInterface.h"

#include "Component/Climb/ZCCharacterMovementComponent.h"
#include "Component/Inventory/ZCInventoryComponent.h"
#include "Component/Device/ZCPlayerDeviceComponent.h"
#include "Component/Device/ZCDeviceSkillComponent.h"
#include "Component/Device/Skill/ZCUltrahandSkillComponent.h"

#include "Development/ZCLogger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCPlayerController)

AZCPlayerController::AZCPlayerController()
{
	DeviceControlComponent = CreateDefaultSubobject<UZCPlayerDeviceComponent>(TEXT("DeviceControlComponent"));

	UltrahandSkillComponent = CreateDefaultSubobject<UZCUltrahandSkillComponent>(TEXT("UltrahandComponent"));

	GrapComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("Grap"));
}

void AZCPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UltrahandSkillComponent->SetPlayerController(this);
	UltrahandSkillComponent->SetPhysicsHandleComponent(GrapComponent);
	UltrahandSkillComponent->OnDeactivate.AddUObject(this, &AZCPlayerController::DeactiveSkill);
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
	SetPlayerInputMappingContext(InUIMappingContext, 0);
}

void AZCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	DeviceControlComponent->InitWidget();

	ZCPlayerCharacter = Cast<AZCPlayerCharacter>(InPawn);
	if (ZCPlayerCharacter)
	{
		if (UZCCharacterMovementComponent* MovementComp = ZCPlayerCharacter->GetZCCharacterMovementComponent())
		{
			MovementComp->OnMovementChangeDelegate.AddDynamic(this, &AZCPlayerController::OnMovementChange);
		}
	}

	if (UCommonInputSubsystem* CIS = ULocalPlayer::GetSubsystem<UCommonInputSubsystem>(GetLocalPlayer()))
	{
		CIS->OnInputMethodChangedNative.AddUObject(this, &AZCPlayerController::HandleInputMethod);
	}

}

void AZCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(HorseCallAction, ETriggerEvent::Started, this, &AZCPlayerController::HorseCall);

		EnhancedInputComponent->BindAction(ShortCutScollAction, ETriggerEvent::Ongoing, this, &AZCPlayerController::ShortCutScroll);
		EnhancedInputComponent->BindAction(ShortCutAction, ETriggerEvent::Started, this, &AZCPlayerController::ShowShortCut);
		EnhancedInputComponent->BindAction(ShortCutAction, ETriggerEvent::Completed, this, &AZCPlayerController::HideShortCut);

		EnhancedInputComponent->BindAction(InteraactionAction, ETriggerEvent::Triggered, this, &AZCPlayerController::Interaction);

		EnhancedInputComponent->BindAction(RightHandAction, ETriggerEvent::Canceled, this, &AZCPlayerController::LastRightHandSkill);
		EnhancedInputComponent->BindAction(RightHandAction, ETriggerEvent::Triggered, this, &AZCPlayerController::SelectRightHandSkill);
		EnhancedInputComponent->BindAction(RightHandAction, ETriggerEvent::Completed, this, &AZCPlayerController::UseRightHandSkill);

		EnhancedInputComponent->BindAction(ChoseRightHandSkillAction, ETriggerEvent::Ongoing, this, &AZCPlayerController::ChoseRightHandSkill);


		/*================================================================================*/
		UltrahandSkillComponent->SetupInputComponent(EnhancedInputComponent);
	}
}

void AZCPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	ZCPlayerState = Cast<AZCPlayerState>(PlayerState);
}

void AZCPlayerController::SetPlayerInputMappingContext(UInputMappingContext* NewContext, int32 Priority)
{
	if (!NewContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(NewContext, Priority);
	}
}

void AZCPlayerController::RemovePlayerInputMappingContext(UInputMappingContext* NewContext)
{
	if (!NewContext) return;

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(NewContext);
	}
}

void AZCPlayerController::Interaction()
{
	// 현재 활성화된 플래그 중 가장 우선순위가 높은 이벤트를 가져옵니다.
	const EFaceButtonEvent TopEvent = GetTopPriorityInteraction();
	// 우선순위에 따라 실제 행동을 실행합니다.
	switch (TopEvent)
	{
	case EFaceButtonEvent::Ambush:
		// TODO: 습격 로직 실행
		break;
	case EFaceButtonEvent::Talk:
		// TODO: 대화 로직 실행
		break;
	case EFaceButtonEvent::Interact:
		// TODO: 상호작용 로직 실행
		break;
	case EFaceButtonEvent::ClimbDown:
		ZCPlayerCharacter->ClimbDownCliff();
		break;
	case EFaceButtonEvent::PickUp:
		PickUpNearItem(); // 아이템 습득 로직 실행
		break;
	case EFaceButtonEvent::None:
	default:
		// 할 수 있는 행동이 없음
		break;
	}
}

void AZCPlayerController::LastRightHandSkill()
{
	//UZCLogger::Warning(TEXT("최신 스킬 선택완료"));
	ActiveSkill(DeviceControlComponent->GetCurrentSkillType());
}

void AZCPlayerController::SelectRightHandSkill()
{
	//UZCLogger::Warning(TEXT("선택화면"));

	SetPlayerInputMappingContext(InUIMappingContext, 2);

	SetPause(true);

	DeviceControlComponent->StartChoseSkill();

}

void AZCPlayerController::UseRightHandSkill()
{
	//UZCLogger::Warning(TEXT("선택완료"));

	SetPlayerInputMappingContext(InUIMappingContext, 0);

	SetPause(false);

	DeviceControlComponent->StopChoseSkill();

	ActiveSkill(DeviceControlComponent->GetCurrentSkillType());
}

void AZCPlayerController::ChoseRightHandSkill(const FInputActionValue& Value)
{
	FVector2D InputPos = Value.Get<FVector2D>();

	DeviceControlComponent->UpdateInputPosition(InputPos);
}

void AZCPlayerController::ShortCutScroll(const FInputActionValue& Value)
{
	const float ScrollValue = Value.Get<float>();
	//UZCLogger::Warning(TEXT("{0}"), ScrollValue);
	int32 Direction = 0;
	if (ScrollValue > 0.0f) Direction = 1;
	else if (ScrollValue < 0.0f) Direction = -1;
	ZCHUDWidget->ScrollShortCutItemList(Direction);
}

void AZCPlayerController::ShowShortCut(const FInputActionValue& Value)
{
	FVector2D ShortCut2D = Value.Get<FVector2D>();

	if (ShortCut2D.Y > 0.0)
	{
		CurrentShortCutItemType = EItemType::Normal;

	}
	else if (ShortCut2D.X < 0.0)
	{
		CurrentShortCutItemType = EItemType::Shield;
	}
	else
	{
		CurrentShortCutItemType = EItemType::Weapon;
	}

	ZCHUDWidget->ShowShortCutUI(CurrentShortCutItemType);
	SetPause(true);
}

void AZCPlayerController::HideShortCut(const FInputActionValue& Value)
{
	SpawnItem();
	ZCHUDWidget->HideShortCutUI();
	SetPause(false);
}

void AZCPlayerController::HorseCall()
{
	UE_LOG(LogTemp, Warning, TEXT("말 부르기"));
}

void AZCPlayerController::HandleInputMethod(ECommonInputType NewType)
{
	DeviceControlComponent->SetCurrentInput(NewType);
}

void AZCPlayerController::ActiveSkill(ESkillType SkillType)
{
	switch (SkillType)
	{
	case ESkillType::Ultrahand:
	{
		SetPlayerInputMappingContext(UltrahandMappingContext, 0);
		UltrahandSkillComponent->Activate();

		RemovePlayerInputMappingContext(InGameMappingContext);
		RemovePlayerInputMappingContext(InUIMappingContext);
	}
		break;
	case ESkillType::Fuse:
		break;
	case ESkillType::Ascend:
		break;
	case ESkillType::Recall:
		break;
	case ESkillType::Autobuild:
		break;
	case ESkillType::Max:
		return;
	default:
		return;
	}
}

void AZCPlayerController::DeactiveSkill(ESkillType SkillType)
{
	SetPlayerInputMappingContext(InGameMappingContext, 1);
	SetPlayerInputMappingContext(InUIMappingContext, 0);

	switch (SkillType)
	{
	case ESkillType::Ultrahand:
	{
		RemovePlayerInputMappingContext(UltrahandMappingContext);
	}
		break;
	case ESkillType::Fuse:
		break;
	case ESkillType::Ascend:
		break;
	case ESkillType::Recall:
		break;
	case ESkillType::Autobuild:
		break;
	case ESkillType::Max:
		return;
	default:
		return;
	}
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
		switch (CurrentShortCutItemType)
		{
		case EItemType::Normal:
			break;

		case EItemType::Weapon:
			// 인벤토리에서 아이템 가져오기
			ZCPlayerCharacter->SetNewWeapon(ZCPlayerState->GetInventoryComponent()->SpawnWeaponItem(CurrentIndex, FTransform::Identity));
			break;

		case EItemType::Shield:
			// 인벤토리에서 아이템 가져오기
			ZCPlayerCharacter->SetNewShield(ZCPlayerState->GetInventoryComponent()->SpawnShieldItem(CurrentIndex, FTransform::Identity));
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
				ZCPlayerState->GetInventoryComponent()->AddItem(TUniquePtr<FZCItemTable>(static_cast<FZCItemTable*>(Item.Release())));
			}

			// NotifyItemOutRange를 호출 안해도 Item의 Destory가 호출될때 자동으로 호출됨
		}
	}
}

void AZCPlayerController::UpdateHUDInteractionEvent()
{
	if (!ZCHUDWidget) return;

	// 표시할 최상위 이벤트를 결정
	const EFaceButtonEvent EventToDisplay = GetTopPriorityInteraction();

	// HUD 업데이트
	switch (EventToDisplay)
	{
	case EFaceButtonEvent::None:
		ZCHUDWidget->HideFaceButtonUI();
		break;
	case EFaceButtonEvent::PickUp:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("습득")));
		break;
	case EFaceButtonEvent::ClimbDown:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("내려가기")));
		break;
	case EFaceButtonEvent::Interact:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("상호작용")));
		break;
	case EFaceButtonEvent::Talk:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::BButton, FText::FromString(TEXT("대화")));
		break;
	case EFaceButtonEvent::Ambush:
		ZCHUDWidget->ShowFaceButtonUI(EFaceButtonType::YButton, FText::FromString(TEXT("습격")));
		break;
	default:
		ZCHUDWidget->HideFaceButtonUI();
		break;
	}
}

void AZCPlayerController::AddInteractionEvent(const EFaceButtonEvent NewEvent)
{
	if (NewEvent == EFaceButtonEvent::None) return;

	// 현재 플래그에 NewEvent가 없는 경우에만 추가하고 업데이트
	if (!EnumHasAnyFlags(CurrentInteractionFlags, NewEvent))
	{
		// 비트 OR 연산으로 플래그 추가
		CurrentInteractionFlags |= NewEvent;
		UpdateHUDInteractionEvent(); // HUD 업데이트
	}
}

void AZCPlayerController::RemoveInteractionEvent(const EFaceButtonEvent RemoveEvent)
{
	if (RemoveEvent == EFaceButtonEvent::None) return;

	// 현재 플래그에 RemoveEvent가 있는 경우에만 제거하고 업데이트
	if (EnumHasAnyFlags(CurrentInteractionFlags, RemoveEvent))
	{
		// 비트 AND NOT 연산으로 플래그 제거
		CurrentInteractionFlags &= ~RemoveEvent;
		UpdateHUDInteractionEvent(); // HUD 업데이트
	}
}

void AZCPlayerController::ClearInteractionEvent()
{
	if (CurrentInteractionFlags != EFaceButtonEvent::None)
	{
		CurrentInteractionFlags = EFaceButtonEvent::None;
		UpdateHUDInteractionEvent(); // HUD 업데이트
	}
}

void AZCPlayerController::OnMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode)
{
	DeactiveSkill(DeviceControlComponent->GetCurrentSkillType());

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

EFaceButtonEvent AZCPlayerController::GetTopPriorityInteraction() const
{
	if (EnumHasAnyFlags(CurrentInteractionFlags, EFaceButtonEvent::Ambush))
	{
		return EFaceButtonEvent::Ambush;
	}
	if (EnumHasAnyFlags(CurrentInteractionFlags, EFaceButtonEvent::Talk))
	{
		return EFaceButtonEvent::Talk;
	}
	if (EnumHasAnyFlags(CurrentInteractionFlags, EFaceButtonEvent::Interact))
	{
		return EFaceButtonEvent::Interact;
	}
	if (EnumHasAnyFlags(CurrentInteractionFlags, EFaceButtonEvent::ClimbDown))
	{
		return EFaceButtonEvent::ClimbDown;
	}
	if (EnumHasAnyFlags(CurrentInteractionFlags, EFaceButtonEvent::PickUp))
	{
		return EFaceButtonEvent::PickUp;
	}

	return EFaceButtonEvent::None;
}
