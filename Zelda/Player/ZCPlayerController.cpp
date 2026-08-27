// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZCPlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "CommonInputSubsystem.h"

#include "UI/HUD/ZCHUDWidget.h"
#include "UI/HUD/SubWidget/FaceButton/ZCFaceButtonWidget.h"

#include "Character/Player/ZCPlayerCharacter.h"
#include "Player/ZCPlayerState.h"

#include "Interface/ZCItemInterface.h"

#include "Player/ZCHUDLocalPlayerSubsystem.h"

#include "Component/LockOn/ZCLockSpringArmComponent.h"
#include "Component/Movement/ZCCharacterMovementComponent.h"
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

	SetPlayerInputMappingContext(InGameMappingContext, 1);
	SetPlayerInputMappingContext(InUIMappingContext, 0);
}

void AZCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	DeviceControlComponent->InitWidget();

	ZCPlayerCharacter = Cast<AZCPlayerCharacter>(InPawn);

	if (UZCCharacterMovementComponent* MoveComp = ZCPlayerCharacter->GetZCCharacterMovementComponent())
	{
		MoveComp->OnMovementChangeDelegate.AddDynamic(this, &AZCPlayerController::OnPlayerMovementChange);
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

		EnhancedInputComponent->BindAction(RideOffAction, ETriggerEvent::Triggered, this, &AZCPlayerController::RideOffGear);

		/*================================================================================*/
		UltrahandSkillComponent->SetupInputComponent(EnhancedInputComponent);
	}
}

void AZCPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	ZCPlayerState = Cast<AZCPlayerState>(PlayerState);
}

void AZCPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		HUDLocalPlayerSubsystem = LocalPlayer->GetSubsystem<UZCHUDLocalPlayerSubsystem>();

		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::PickUp, EFaceButtonType::BButton, FText::FromString(TEXT("습득")));
		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::ClimbDown, EFaceButtonType::BButton, FText::FromString(TEXT("내려가기")));
		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::Interact, EFaceButtonType::BButton, FText::FromString(TEXT("상호작용")));
		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::Talk, EFaceButtonType::BButton, FText::FromString(TEXT("대화")));
		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::Ambush, EFaceButtonType::XButton, FText::FromString(TEXT("습격")));

		HUDLocalPlayerSubsystem->AddFaceButtonContext(EZCFaceButtonEvent::RideGear, EFaceButtonType::BButton, FText::FromString(TEXT("승차")));

		HUDLocalPlayerSubsystem->SetHUDModeState(EZCHUDMode::Normal);

		UltrahandSkillComponent->SetHUDSubsystem(HUDLocalPlayerSubsystem);
	}
}

void AZCPlayerController::SetPlayerVFX(UZCNiagaraComponent* NewVFX)
{
	ZCPlayerVFXComponent = NewVFX;

	UltrahandSkillComponent->SetPlayerVFXComponent(ZCPlayerVFXComponent);
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
	const EZCFaceButtonEvent TopEvent = HUDLocalPlayerSubsystem->GetCurrentFaceButtonEvent();

	switch (TopEvent)
	{
	case EZCFaceButtonEvent::PickUp:
		PickUpNearItem();
		break;
	case EZCFaceButtonEvent::ClimbDown:
		ZCPlayerCharacter->ClimbDownCliff();
		break;
	case EZCFaceButtonEvent::Ambush:
		break;
	case EZCFaceButtonEvent::Interact:
		break;
	case EZCFaceButtonEvent::Talk:
		break;
	case EZCFaceButtonEvent::RideGear:
		RideOnGear();
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
	HUDLocalPlayerSubsystem->ScrollShortCutItemList(ScrollValue);
}

void AZCPlayerController::ShowShortCut(const FInputActionValue& Value)
{
	FVector2D ShortCut2D = Value.Get<FVector2D>();
	HUDLocalPlayerSubsystem->ShowShortCutUI(ShortCut2D);
	SetPause(true);
}

void AZCPlayerController::HideShortCut(const FInputActionValue& Value)
{
	SpawnItemFromShortCut();
	HUDLocalPlayerSubsystem->HideShortCutUI();
	SetPause(false);
}

void AZCPlayerController::HorseCall()
{
	UE_LOG(LogTemp, Warning, TEXT("말 부르기"));
}

void AZCPlayerController::HandleInputMethod(ECommonInputType NewType)
{
	// 현재 입력 모드 전파
	DeviceControlComponent->SetCurrentInput(NewType);
}

void AZCPlayerController::ActiveSkill(ESkillType SkillType)
{
	HUDLocalPlayerSubsystem->SetHUDModeState(EZCHUDMode::Skill);

	ZCPlayerCharacter->GetCameraBoom()->ChangeSocketOffsetDelta(FVector(0.f, 0.f, 100.f));

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
	HUDLocalPlayerSubsystem->SetHUDModeState(EZCHUDMode::Normal);

	ZCPlayerCharacter->GetCameraBoom()->RestoreSocketOffset();

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

	HUDLocalPlayerSubsystem->ShowFaceButtonEvent(EZCFaceButtonEvent::PickUp);
}

void AZCPlayerController::NotifyItemOutRange(AActor* ItemActor)
{
	ItemInRangeSet.Remove(ItemActor);
	if (ItemInRangeSet.IsEmpty())
	{
		HUDLocalPlayerSubsystem->HideFaceButtonEvent(EZCFaceButtonEvent::PickUp);
	}
}

void AZCPlayerController::SpawnItemFromShortCut()
{
	int32 CurrentIndex = -1;
	EItemType CurrentType = EItemType::None;

	HUDLocalPlayerSubsystem->GetCurrentShortCutSelection(CurrentIndex, CurrentType);

	UZCInventoryComponent* InventoryComp = ZCPlayerState->GetInventoryComponent();
	if (!InventoryComp) return;

	if (CurrentIndex == -1)
	{
		switch (CurrentType)
		{
		case EItemType::Normal: break;
		case EItemType::Weapon:
			ZCPlayerCharacter->SetNewWeapon(nullptr);
			break;
		case EItemType::Shield:
			ZCPlayerCharacter->SetNewShield(nullptr);
			break;
		default: break;
		}
	}
	else
	{
		switch (CurrentType)
		{
		case EItemType::Normal:
			break;

		case EItemType::Weapon:
			ZCPlayerCharacter->SetNewWeapon(InventoryComp->SpawnWeaponItem(CurrentIndex, FTransform::Identity));
			break;

		case EItemType::Shield:
			ZCPlayerCharacter->SetNewShield(InventoryComp->SpawnShieldItem(CurrentIndex, FTransform::Identity));
			break;

		default: break;
		}
	}
}

void AZCPlayerController::PickUpNearItem()
{
	if (ItemInRangeSet.IsEmpty()) return;

	const FVector PlayerLocation = GetPawn()->GetActorLocation();

	AActor* ClosestItem = nullptr;

	float MinXYDistSquared = FLT_MAX;
	float MinZDistance = FLT_MAX;

	for (auto It = ItemInRangeSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		AActor* ItemActor = It->Get();
		const FVector ItemLocation = ItemActor->GetActorLocation();

		float XYDistSquared = FVector::DistSquaredXY(PlayerLocation, ItemLocation);
		float ZDistance = FMath::Abs(PlayerLocation.Z - ItemLocation.Z);

		if (XYDistSquared < MinXYDistSquared)
		{
			MinXYDistSquared = XYDistSquared;
			MinZDistance = ZDistance;
			ClosestItem = ItemActor;
		}
		else if (FMath::IsNearlyEqual(XYDistSquared, MinXYDistSquared))
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
		ItemInRangeSet.Remove(ClosestItem);

		if (IZCItemInterface* AddItem = Cast<IZCItemInterface>(ClosestItem))
		{
			if (TUniquePtr<FZCItemTable> Item = AddItem->OnItemPickUp())
			{
				if (ZCPlayerState && ZCPlayerState->GetInventoryComponent())
				{
					ZCPlayerState->GetInventoryComponent()->AddItem(TUniquePtr<FZCItemTable>(static_cast<FZCItemTable*>(Item.Release())));
				}
			}

			// NotifyItemOutRange 호출은 Item의 Destory가 호출될때 자동으로 호출됨
		}
	}
}

void AZCPlayerController::RideOnGear()
{
	HUDLocalPlayerSubsystem->SetHUDModeState(EZCHUDMode::Gear);

	SetPlayerInputMappingContext(RideMappingContext, 0);

	RemovePlayerInputMappingContext(InGameMappingContext);
	RemovePlayerInputMappingContext(InUIMappingContext);

	ZCPlayerCharacter->RideOnGear();
}

void AZCPlayerController::RideOffGear()
{
	HUDLocalPlayerSubsystem->SetHUDModeState(EZCHUDMode::Normal);

	SetPlayerInputMappingContext(InGameMappingContext, 1);
	SetPlayerInputMappingContext(InUIMappingContext, 0);

	RemovePlayerInputMappingContext(RideMappingContext);

	ZCPlayerCharacter->RideOffGear();
}

void AZCPlayerController::OnPlayerMovementChange(const EMovementMode NewMovementMode, const ECustomMovementMode NewCustomMovementMode)
{
}
