// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Device/ZCPlayerDeviceComponent.h"
#include "GameData/Struct/ZCWheelStruct.h"
#include "UI/HUD/SubWidget/Wheel/ZCWheelWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "Kismet/KismetMathLibrary.h"

#include "Development/ZCLogger.h"

#include "Component/Device/Skill/ZCUltrahandSkillComponent.h"


// Sets default values for this component's properties
UZCPlayerDeviceComponent::UZCPlayerDeviceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// ...
}

// Called every frame
void UZCPlayerDeviceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UZCPlayerDeviceComponent::InitWidget()
{
	WheelWidget = CreateWidget<UZCWheelWidget>(CastChecked<APlayerController>(GetOwner()), WheelWidgetClass);
	if (WheelWidget)
	{
		WheelWidget->SetShowMouseCursor(bShowCurserWhenUsingMouse);
		WheelWidget->SetAnimationDuration(ShowAnimationDuration, HideAnimationDuration);
		WheelWidget->SetOpenSound(OpenSound, OpenSoundVolume);
		WheelWidget->SetCloseSound(ClosedSound, CloseSoundVolume);

		WheelWidget->AddToViewport(2);

		WheelWidget->GenerateRadialWheel(SkillSlot, RotationOffset);
		WheelWidget->RefreshRadialWheelStyle(WheelPos, WheelBorderColor, WheelBackGroundImage, WheelBackgroundColor, WheelBackgroundOffset,	WheelSlotColor, TextTransformPolicy, WheelSelectTextOffset, WheelSelectionTextShadowColor, WheelSelectTextShadowOffset);

		WheelWidget->SetVisibility(ESlateVisibility::Collapsed);

		WheelGeometry = &WheelWidget->GetWheelOverlayGeometry();
	}
}

void UZCPlayerDeviceComponent::StartChoseSkill()
{
	if (!WheelWidget->IsWheelOpen())
	{
		WheelWidget->ShowWheel();
	}
}

void UZCPlayerDeviceComponent::StopChoseSkill()
{
	if (WheelWidget->IsWheelOpen())
	{
		CurrentIdx = WheelWidget->GetCurentSlotIdx();

		CurrentSkillType = SkillSlotIdxToEnum(CurrentIdx);

		WheelWidget->HideWheel();
	}
}

void UZCPlayerDeviceComponent::UpdateInputPosition(FVector2D& InputPos)
{
	switch (CurrentInputType)
	{
	case ECommonInputType::MouseAndKeyboard:	
		CalculateMousePos(InputPos);
		break;
	case ECommonInputType::Gamepad:
		CalculateGamepadPos(InputPos);
		break;
	}
}

void UZCPlayerDeviceComponent::CalculateMousePos(FVector2D& InputPos)
{
	// 마우스는 입력값 사용 못하므로 덮어씌움
	InputPos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	FVector2D PixelPosition, ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(this, *WheelGeometry, WheelGeometry->GetLocalSize() / 2.0f, PixelPosition, ViewportPosition);

	float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);

	// 마우스와 휠 중심 간의 상대 벡터 계산
	FVector2D Result = InputPos * ViewportScale - ViewportPosition * ViewportScale;
	float Distance = Result.Length();

	Result = Result.GetSafeNormal();

	// Atan2는 라디안을 반환하므로 Degrees로 변환
	float Atan2Radians = FMath::Atan2(Result.Y, Result.X);
	float AngleDegrees = FMath::RadiansToDegrees(Atan2Radians);

	// 90도 회전 적용 (12시 방향을 0도로 만들기 위해)
	AngleDegrees += 90.0f;

	// RotationOffset 적용
	AngleDegrees -= RotationOffset;

	// 0~360 범위로 정규화
	float Angle = UKismetMathLibrary::ClampAxis(AngleDegrees);

	// 값이 너무 작으면 무시
	bool InDeadZone = Distance < MouseDistanceDeadZone;

	WheelWidget->UpdateSelectWheel(Angle, InDeadZone);

}

void UZCPlayerDeviceComponent::CalculateGamepadPos(FVector2D& InputPos)
{
	// 거리 계산 (0~1 범위, 이미 정규화된 입력이므로)
	float Distance = InputPos.Length();

	// 정규화된 방향 벡터
	FVector2D NormalizedInput = InputPos.GetSafeNormal();

	// Atan2로 각도 계산 (라디안)
	float Atan2Radians = FMath::Atan2(NormalizedInput.Y, NormalizedInput.X);
	float AngleDegrees = FMath::RadiansToDegrees(Atan2Radians);

	// 90도 회전 적용 (12시 방향을 0도로)
	AngleDegrees += 90.0f;

	// RotationOffset 적용
	AngleDegrees -= RotationOffset;

	// 0~360 범위로 정규화
	float Angle = UKismetMathLibrary::ClampAxis(AngleDegrees);

	// 입력값이 너무 작으면 무시
	bool InDeadZone = Distance < GamepadStickDeadZone;

	WheelWidget->UpdateSelectWheel(Angle, InDeadZone);
}

ESkillType UZCPlayerDeviceComponent::SkillSlotIdxToEnum(int32 Idx)
{
	switch (Idx)
	{
	case 0 : return ESkillType::Ascend;
	case 2 : return ESkillType::Ultrahand;
	case 3 : return ESkillType::Autobuild;
	case 4 : return ESkillType::Fuse;
	case 6 : return ESkillType::Recall;

	default: return ESkillType::Max;
	}
}

