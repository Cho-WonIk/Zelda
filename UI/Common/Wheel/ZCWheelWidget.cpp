// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/Wheel/ZCWheelWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "Styling/SlateColor.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/OverlaySlot.h"

#include "GameData/PrimaryData/Widget/Wheel/ZCWheelSlotStylePrimaryDataAsset.h"
#include "Settings/UI/ZCDefaultWidgetSettings.h"

#include "Component/Device/ZCPlayerDeviceComponent.h"


void UZCWheelItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetRenderTransformAngle(Angle);

	ContentOverlay->SetRenderTransformAngle(-(Angle + AngleOffset));
}

void UZCWheelItemWidget::UpdateSlot(FWheelSlot& NewSlot)
{
	SlotData = &NewSlot;

	UZCWheelSlotStylePrimaryDataAsset* SelectStyle = SlotData->bIsSelected ? SlotData->SlotActiveStyle : SlotData->SlotStyle;

	if (!SelectStyle)
	{
		const UZCDefaultWidgetSettings* Def = GetDefault<UZCDefaultWidgetSettings>();
		SelectStyle = SlotData->bIsSelected ? Def->DefaultSelectWheelSetting.LoadSynchronous() : Def->DefaultWheelSetting.LoadSynchronous();
	}

	UpdateSlotStyle(SelectStyle);
	
	if (SlotData->bHasItem)
	{
		FWheelItem& SelectItem = SlotData->Item;

		ContentOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		ItemIconImage->SetBrushResourceObject(SelectItem.Icon);
		ItemIconImage->SetDesiredSizeOverride(SelectItem.IconSize);
		ItemIconImage->SetRenderTransform(SelectItem.IconTransform);

		ItemInfoText->SetText(SelectItem.InfoText);
		ItemInfoText->SetRenderTransform(SelectItem.InfoTextTransform);
		ItemInfoText->SetColorAndOpacity(FSlateColor(SelectStyle->InfoTextColor));

		ItemInfoText->SetFont(SelectStyle->InfoTextFont);
		ItemInfoText->SetTextTransformPolicy(SelectStyle->InfoTextTransformPolicy);

		ItemInfoText->SetShadowColorAndOpacity(SelectStyle->InfoTextShadowColor);
		ItemInfoText->SetShadowOffset(SelectStyle->InfoTextShadowOffset);
	}
	else
	{
		ContentOverlay->SetVisibility(ESlateVisibility::Collapsed);
		if (SelectStyle)
		{
			WheelItemImage->SetBrushTintColor(SelectStyle->EmptySlotColorOverlay);
		}
		else
		{
			WheelItemImage->SetBrushTintColor(FLinearColor::Transparent);
		}
	}
}

void UZCWheelItemWidget::UpdateSlotStyle(UZCWheelSlotStylePrimaryDataAsset* NewStyle)
{
	if (NewStyle == CurrentSlotStyle) return;

	CurrentSlotStyle = NewStyle;

	WheelMaterialInstance = CreateMaterialInstanceDynamic(CurrentSlotStyle);
	WheelItemImage->SetBrushResourceObject(WheelMaterialInstance);

	if (CurrentSlotStyle)
	{
		ContentOverlay->SetRenderTranslation(FVector2D(0.0f, -CurrentSlotStyle->ContentOffset));
		ItemIconImage->SetColorAndOpacity(CurrentSlotStyle->IconColor);
	}
	else
	{
		ContentOverlay->SetRenderTranslation(FVector2D(0.0f, -230.0f));
		ItemIconImage->SetColorAndOpacity(FLinearColor::White);
	}
}

UMaterialInstanceDynamic* UZCWheelItemWidget::CreateMaterialInstanceDynamic(UZCWheelSlotStylePrimaryDataAsset* Style)
{
	if (!Style) return nullptr;

	UMaterialInstanceDynamic* NewMID = nullptr;
	if (Style->SlotType == EWheelSlotType::CustomImage)
	{
		NewMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, CustomImageMaterial);
		NewMID->SetScalarParameterValue(FName(TEXT("Scale")), Scale);
		NewMID->SetScalarParameterValue(FName(TEXT("Image Angle")), UKismetMathLibrary::ClampAxis(Angle) / 360.0f);
		NewMID->SetTextureParameterValue(FName(TEXT("Image")), Style->Image);
		NewMID->SetScalarParameterValue(FName(TEXT("Padding")), Style->Padding);
		NewMID->SetVectorParameterValue(FName(TEXT("Color")), Style->ImageColor);
		MainSizeBox->SetWidthOverride(Style->ImageSize.X);
		MainSizeBox->SetHeightOverride(Style->ImageSize.Y);
	}
	else if (Style->SlotType == EWheelSlotType::Procedural)
	{
		const bool InnerBorder = Style->bShowInnerBorder;
		const bool OuterBorder = Style->bShowOuterBorder;
		//	// 양쪽, 안쪽, 바깥쪽, 없음 머티리얼 선택로직
		UMaterialInterface* MatToUse = InnerBorder ? (OuterBorder ? BothBorderMaterial : InnerBorderMaterial) : (OuterBorder ? OuterBorderMaterial : NoBorderMaterial);

		NewMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, MatToUse);

		NewMID->SetScalarParameterValue(FName(TEXT("Scale")), Scale);

		MainSizeBox->SetWidthOverride(1080.0f);
		MainSizeBox->SetHeightOverride(1080.0f);

		NewMID->SetScalarParameterValue(FName(TEXT("Radius")), Style->Radius);
		NewMID->SetScalarParameterValue(FName(TEXT("Thickness")), Style->ProceduralSetting.Thickness);
		NewMID->SetScalarParameterValue(FName(TEXT("Padding")), Style->Padding);
		NewMID->SetScalarParameterValue(FName(TEXT("Density")), Style->ProceduralSetting.Density);
		NewMID->SetVectorParameterValue(FName(TEXT("Color")), Style->ProceduralSetting.Color);
		
		if (InnerBorder)
		{
			NewMID->SetScalarParameterValue(FName(TEXT("Inner Border Thickness")), Style->InnerBorderSetting.Thickness);
			NewMID->SetScalarParameterValue(FName(TEXT("Inner Border Density")), Style->InnerBorderSetting.Density);
			NewMID->SetScalarParameterValue(FName(TEXT("Inner Border Offset")), Style->InnerBorderSetting.Offset);
			NewMID->SetVectorParameterValue(FName(TEXT("Inner Border Color")), Style->InnerBorderSetting.Color);
		}

		if (OuterBorder)
		{
			NewMID->SetScalarParameterValue(FName(TEXT("Outer Border Thickness")), Style->OuterBorderSetting.Thickness);
			NewMID->SetScalarParameterValue(FName(TEXT("Outer Border Density")), Style->OuterBorderSetting.Density);
			NewMID->SetScalarParameterValue(FName(TEXT("Outer Border Offset")), Style->OuterBorderSetting.Offset);
			NewMID->SetVectorParameterValue(FName(TEXT("Outer Border Color")), Style->OuterBorderSetting.Color);
		}
	}

	return NewMID;
}

void UZCWheelItemWidget::OnSelected()
{
	SlotData->bIsSelected = true;

	UZCWheelSlotStylePrimaryDataAsset* SelectStyle = SlotData->bIsSelected ? SlotData->SlotActiveStyle : SlotData->SlotStyle;

	if (!SelectStyle)
	{
		const UZCDefaultWidgetSettings* Def = GetDefault<UZCDefaultWidgetSettings>();
		SelectStyle = SlotData->bIsSelected ? Def->DefaultSelectWheelSetting.LoadSynchronous() : Def->DefaultWheelSetting.LoadSynchronous();
	}

	UpdateSlotStyle(SelectStyle);

	PlayAnimationForward(Selected, (1.0f / CurrentSlotStyle->SelectedAnimationDuration));
	UGameplayStatics::PlaySound2D(this, CurrentSlotStyle->SelectedSlotSound, CurrentSlotStyle->SelectedSlotSoundVolume);
}

void UZCWheelItemWidget::OnUnSelected()
{
	SlotData->bIsSelected = false;

	UZCWheelSlotStylePrimaryDataAsset* SelectStyle = SlotData->bIsSelected ? SlotData->SlotActiveStyle : SlotData->SlotStyle;

	if (!SelectStyle)
	{
		const UZCDefaultWidgetSettings* Def = GetDefault<UZCDefaultWidgetSettings>();
		SelectStyle = SlotData->bIsSelected ? Def->DefaultSelectWheelSetting.LoadSynchronous() : Def->DefaultWheelSetting.LoadSynchronous();
	}

	UpdateSlotStyle(SelectStyle);

	PlayAnimationReverse(Selected, (1.0f / CurrentSlotStyle->UnselectedAnimationDuration));
	UGameplayStatics::PlaySound2D(this, CurrentSlotStyle->UnselectedSlotSound, CurrentSlotStyle->UnselectedSlotSoundVolume);
}

void UZCWheelItemWidget::OnSelectedAnimationRepeater()
{
	FVector2D RenderTranslation = FVector2D(0.0f, -UKismetMathLibrary::Lerp(CurrentSlotStyle->VerticalOffset, CurrentSlotStyle->SelectedVerticalOffset, GetAnimationCurrentTime(Selected)));
	MainSizeBox->SetRenderTranslation(RenderTranslation);
	ContentOverlay->SetRenderTranslation(RenderTranslation);

	FLinearColor RenderColorAndOpacity = UKismetMathLibrary::LinearColorLerp(CurrentSlotStyle->IconColor, CurrentSlotStyle->SelectedIconColor, GetAnimationCurrentTime(Selected));

	ItemIconImage->SetColorAndOpacity(RenderColorAndOpacity);
	ItemInfoText->SetColorAndOpacity(FSlateColor(RenderColorAndOpacity));
}

/*==================================================================================================================================================================*/

void UZCWheelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		GenerateRadialWheel(PreSlot, 0.0f);
	}
}

void UZCWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UZCWheelWidget::GenerateRadialWheel(TArray<FWheelSlot>& InSlot, float RotationOffset)
{
	WheelOverlay->ClearChildren();

	const int32 Count = InSlot.Num();
	WidgetSlots.Reset(Count);
	if (Count <= 0) return;

	RotationOffset = UKismetMathLibrary::NormalizeAxis(RotationOffset);

	// 균등 분할을 위한 각도 계산
	const float AnglePerSlot = 360.0f / Count;

	for (int32 i = 0; i < Count; ++i)
	{
		// 각 슬롯의 중심 각도 계산 (균등 분할)
		const float centerDeg = i * AnglePerSlot;

		UZCWheelItemWidget* ItemWidget = CreateWidget<UZCWheelItemWidget>(this, SlotWidgetClass);

		// Scale은 균등 분할이므로 모두 동일
		ItemWidget->Scale = 100.0f / Count;
		ItemWidget->Angle = UKismetMathLibrary::NormalizeAxis(centerDeg);
		ItemWidget->AngleOffset = RotationOffset;

		WidgetSlots.Add(ItemWidget);

		UOverlaySlot* OverlaySlot = WheelOverlay->AddChildToOverlay(ItemWidget);
		OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

		ItemWidget->UpdateSlot(InSlot[i]);
	}

	ContainerOverlay->SetRenderTransformAngle(RotationOffset);
	CurrentSlotNameText->SetRenderTransformAngle(-RotationOffset);
}

void UZCWheelWidget::RefreshRadialWheelSlots(TArray<FWheelSlot>& NewSlot)
{
	for (int32 index = 0; index < WidgetSlots.Num(); ++index)
	{
		WidgetSlots[index]->UpdateSlot(NewSlot[index]);

		if (NewSlot[index].bIsSelected) CurrentSlotNameText->SetText(NewSlot[index].Item.InfoText);
	}
}

void UZCWheelWidget::RefreshRadialWheelStyle(EWheelPosition NewWheelPos, FLinearColor BorderColor, UObject* BGImage, FLinearColor BackgroundColor, FVector2D BackgroundOffset, 
												FSlateColor CurrentSlotColor, ETextTransformPolicy CurrentSlotPolicy, 
													FWidgetTransform SelectTextOffset, FLinearColor SelectionTextShadowColor, FVector2D SelectTextShadowOffset)
{
	SetWheelPosition(NewWheelPos);

	BackgroundBorder->SetBrushColor(BorderColor);

	RefreshBackgroundImage(BGImage, BackgroundColor, BackgroundOffset);
	RefreshCurrentSlotText(CurrentSlotColor, CurrentSlotPolicy, SelectTextOffset, SelectionTextShadowColor, SelectTextShadowOffset);

}

void UZCWheelWidget::UpdateSelectWheel(float Angle, bool bInDeadZone)
{
	// 데드존이면 반응 안함
	if (bInDeadZone) return;

	UZCWheelItemWidget* NewWidget = nullptr;

	int32 Idx = 0;
	for (auto *Widget : WidgetSlots)
	{
		if (Widget->SlotData->bHasItem && CheckWheelItemAngle(Widget, Angle))
		{
			NewWidget = Widget;
			CurrentSlotIdx = Idx;
			break;
		}
		++Idx;
	}

	if (CurrentSlot == NewWidget) return;
	if (CurrentSlot) CurrentSlot->OnUnSelected();

	if (NewWidget)
	{
		NewWidget->OnSelected();
		CurrentSlotNameText->SetText(NewWidget->SlotData->Item.DisplayName);
	}

	CurrentSlot = NewWidget;
}

const FGeometry& UZCWheelWidget::GetWheelOverlayGeometry() const
{
	return WheelOverlay->GetTickSpaceGeometry();
}

void UZCWheelWidget::RefreshBackgroundImage(UObject* BGImage, FLinearColor BackgroundColor, FVector2D BackgroundOffset)
{
	BackgroundImage->SetBrushResourceObject(BGImage);
	BackgroundImage->SetColorAndOpacity(BackgroundColor);
	BackgroundImage->SetDesiredSizeOverride(BackgroundOffset);
}

void UZCWheelWidget::RefreshCurrentSlotText(FSlateColor CurrentSlotColor, ETextTransformPolicy CurrentSlotPolicy, FWidgetTransform SelectTextOffset, FLinearColor SelectionTextShadowColor, FVector2D SelectTextShadowOffset)
{
	CurrentSlotNameText->SetColorAndOpacity(CurrentSlotColor);
	CurrentSlotNameText->SetTextTransformPolicy(CurrentSlotPolicy/*Select Text Transform Policy*/);
	CurrentSlotNameText->SetRenderTransform(SelectTextOffset/*Select Text Offset*/);
	CurrentSlotNameText->SetShadowColorAndOpacity(SelectionTextShadowColor/*Selection Text Shadow Color*/);
	CurrentSlotNameText->SetShadowOffset(SelectTextShadowOffset/*Select Text Shadow Offset*/);
}

void UZCWheelWidget::SetWheelPosition(EWheelPosition NewWheelPos)
{
	UCanvasPanelSlot* OVSlot = Cast<UCanvasPanelSlot>(ContainerOverlay->Slot);

	FAnchors NewAnchor;
	FVector2D NewAlignment(0.0f, 0.0f);

	switch (NewWheelPos)
	{
	case EWheelPosition::Center:
	{
		NewAnchor.Minimum = FVector2D(0.5f, 0.5f);
		NewAnchor.Maximum = FVector2D(0.5f, 0.5f);

		NewAlignment.X = 0.5f;
		NewAlignment.Y = 0.5f;
	}
		break;
	case EWheelPosition::Left:
	{
		NewAnchor.Minimum = FVector2D(0.0f, 0.5f);
		NewAnchor.Maximum = FVector2D(0.0f, 0.5f);

		NewAlignment.X = 0.0f;
		NewAlignment.Y = 0.5f;
	}
		break;
	case EWheelPosition::Right:
	{
		NewAnchor.Minimum = FVector2D(1.0f, 0.5f);
		NewAnchor.Maximum = FVector2D(1.0f, 0.5f);

		NewAlignment.X = 1.0f;
		NewAlignment.Y = 0.5f;
	}
		break;
	}

	OVSlot->SetAnchors(NewAnchor);
	OVSlot->SetPosition(FVector2D(0.0f, 0.0f));
	OVSlot->SetAlignment(NewAlignment);
}

bool UZCWheelWidget::GetWheelCenterPosition(int32& X, int32& Y)
{
	const FGeometry& WheelOVGeometry = WheelOverlay->GetTickSpaceGeometry();
	UE::Slate::FDeprecateVector2DResult WheelLocalSize = WheelOVGeometry.GetLocalSize();

	// 생성되었는지 확인
	bool bIsGenerated = WheelLocalSize != UE::Slate::FDeprecateVector2DResult(0.0f, 0.0f);

	FVector2D PixelPosition, ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(this, WheelOVGeometry, WheelLocalSize / 2.0f, PixelPosition, ViewportPosition);

	FVector2D ScaledViewportPos = ViewportPosition * UWidgetLayoutLibrary::GetViewportScale(this);

	X = FMath::TruncToInt(ScaledViewportPos.X);
	Y = FMath::TruncToInt(ScaledViewportPos.Y);

	return bIsGenerated;
}

void UZCWheelWidget::CenterMousePosition()
{
	int32 X, Y;

	// 아직 위젯 생성이 안되었을때 기다리는 로직은 뺌
	if (GetWheelCenterPosition(X, Y))
	{
		GetOwningPlayer()->SetMouseLocation(X, Y);

		GetOwningPlayer()->bShowMouseCursor = bShowCursor;

	}
}

bool UZCWheelWidget::CheckWheelItemAngle(UZCWheelItemWidget* SlotItem, float CurrentAngle)
{
	// 아이템이 차지하는 각도의 절반
	float HalfAngle = SlotItem->Scale / 100.0f * 360.0f / 2.0f;

	// 아이템 중심 각도
	float ItemAngle = FMath::Fmod(SlotItem->GetRenderTransformAngle(), 360.0f);
	if (ItemAngle < 0.0f)
		ItemAngle += 360.0f;

	// 아이템의 시작/끝 각도
	float StartAngle = ItemAngle - HalfAngle;
	float EndAngle = ItemAngle + HalfAngle;

	// 0~360 범위 보정
	StartAngle = FMath::Fmod(StartAngle + 360.0f, 360.0f);
	EndAngle = FMath::Fmod(EndAngle + 360.0f, 360.0f);

	// 0~360을 넘기는 경우(예: 350~10)
	if (StartAngle > EndAngle)
	{
		// 두 구간 중 하나라도 포함되면 true
		return (CurrentAngle >= StartAngle && CurrentAngle <= 360.0f) || (CurrentAngle >= 0.0f && CurrentAngle <= EndAngle);
	}
	else
	{
		return (CurrentAngle >= StartAngle && CurrentAngle <= EndAngle);
	}
}

void UZCWheelWidget::ShowWheel()
{
	bWheelOpen = true;

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	APlayerController* PC = GetOwningPlayer();

	//UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PC, nullptr, EMouseLockMode::DoNotLock, false, false);

	PC->bShowMouseCursor = bShowCursor;

	PlayAnimationForward(PopUp, (1.0f / ShowAnimationDuration));

	CenterMousePosition();

	PlayOpenCloseSound(false);

	// 원본에는 델리게이트 호출이 있음
}

void UZCWheelWidget::HideWheel()
{
	bWheelOpen = false;

	APlayerController* PC = GetOwningPlayer();

	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;

	PlayAnimationReverse(PopUp, (1.0f / HideAnimationDuration));

	PlayOpenCloseSound(true);

	if (CurrentSlot)
	{
		CurrentSlot->OnUnSelected();

	}

	CurrentSlot = nullptr;
	CurrentSlotIdx = -1;

	SetVisibility(ESlateVisibility::Collapsed);
}

void UZCWheelWidget::PlayOpenCloseSound(bool bIsClose)
{
	/*if (TempCurrentClosedOpenedSound)
	{
		TempCurrentClosedOpenedSound->DestroyComponent();
	}*/

	if (!bIsClose)
	{
		// 오픈
		TempCurrentClosedOpenedSound = UGameplayStatics::CreateSound2D(this, OpenSound, OpenSoundVolume);
	}
	else
	{
		// 닫음
		TempCurrentClosedOpenedSound = UGameplayStatics::CreateSound2D(this, ClosedSound, CloseSoundVolume);
	}

	if (TempCurrentClosedOpenedSound)
	{
		TempCurrentClosedOpenedSound->Play();
	}
}
