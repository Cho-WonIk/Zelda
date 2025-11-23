// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameData/Enum/ZCWidget.h"
#include "GameData/Struct/ZCWheelStruct.h"
#include "ZCWheelWidget.generated.h"

class UTextBlock;
class USizeBox;
class UImage;
class UOverlay;
class UWidgetAnimation;
class UMaterialInstanceDynamic;

class UBackgroundBlur;
class UBorder;

class UZCWheelSlotStylePrimaryDataAsset;

class UZCPlayerDeviceComponent;

UCLASS(meta = (DisableNativeTick))
class ZELDA_API UZCWheelItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativePreConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	bool CanBeSelected() { return SlotData->bHasItem; }

	UFUNCTION(BlueprintCallable)
	void UpdateSlot(FWheelSlot& NewSlot);

	UFUNCTION(BlueprintCallable)
	void UpdateSlotStyle(UZCWheelSlotStylePrimaryDataAsset* NewStyle);

	UFUNCTION(BlueprintCallable)
	UMaterialInstanceDynamic* CreateMaterialInstanceDynamic(UZCWheelSlotStylePrimaryDataAsset* Style);

	UFUNCTION(BlueprintCallable)
	void OnSelected();

	UFUNCTION(BlueprintCallable)
	void OnUnSelected();

	UFUNCTION(BlueprintCallable)
	void OnSelectedAnimationRepeater();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> MainSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WheelItemImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ContentOverlay;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	float Scale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	float Angle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	float AngleOffset;

public:
	TObjectPtr<class UZCWheelSlotStylePrimaryDataAsset> CurrentSlotStyle;

	FWheelSlot* SlotData;

	TObjectPtr<UMaterialInstanceDynamic> WheelMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
	TObjectPtr<UWidgetAnimation> Selected;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "커스텀 이미지 머티리얼"))
	TObjectPtr<UMaterialInterface> CustomImageMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "양쪽 경계"))
	TObjectPtr<UMaterialInterface> BothBorderMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "안쪽 경계"))
	TObjectPtr<UMaterialInterface> InnerBorderMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "바깥쪽 경계"))
	TObjectPtr<UMaterialInterface> OuterBorderMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "경계 없음"))
	TObjectPtr<UMaterialInterface> NoBorderMaterial;
};

UCLASS(meta = (DisableNativeTick))
class ZELDA_API UZCWheelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
public:
	// 시작시
	void GenerateRadialWheel(TArray<FWheelSlot>& InSlot, float RotationOffset);

	// 갱신시
	void RefreshRadialWheelSlots(TArray<FWheelSlot>& NewSlot);
	void RefreshRadialWheelStyle(EWheelPosition NewWheelPos, FLinearColor BorderColor, UObject* BGImage, FLinearColor BackgroundColor, FVector2D BackgroundOffset, 
									FSlateColor CurrentSlotColor, ETextTransformPolicy CurrentSlotPolicy, 
										FWidgetTransform SelectTextOffset, FLinearColor SelectionTextShadowColor, FVector2D SelectTextShadowOffset);

	// 마우스 및 게임 패드 위치를 받아서 휠 상태 업데이트
	void UpdateSelectWheel(float Angle, bool bInDeadZone);

	const FGeometry& GetWheelOverlayGeometry() const;

	// 유틸 함수들
protected:
	void RefreshBackgroundImage(UObject* BGImage, FLinearColor BackgroundColor, FVector2D BackgroundOffset);
	void RefreshCurrentSlotText(FSlateColor CurrentSlotColor, ETextTransformPolicy CurrentSlotPolicy, FWidgetTransform SelectTextOffset, FLinearColor SelectionTextShadowColor, FVector2D SelectTextShadowOffset);

	// Wheel의 위치를 정함(중앙, 왼쪽, 오른쪽)
	void SetWheelPosition(EWheelPosition NewWheelPos);
	// Wheel이 생성된 중심 포인트를 계산하여 반환
	bool GetWheelCenterPosition(int32 &X, int32 &Y);
	void CenterMousePosition();

	bool CheckWheelItemAngle(UZCWheelItemWidget* SlotItem, float CurrentAngle);

public:
	UFUNCTION(BlueprintCallable)
	void ShowWheel();
	UFUNCTION(BlueprintCallable)
	void HideWheel();

	UFUNCTION(BlueprintCallable)
	void PlayOpenCloseSound(bool bIsClose);

	UFUNCTION(BlueprintCallable)
	bool IsWheelOpen() { return bWheelOpen; }

	UFUNCTION(BlueprintCallable)
	int32 GetCurentSlotIdx() { return CurrentSlotIdx; }

	// 설정 함수들
public:
	UFUNCTION(BlueprintCallable)
	void SetOpenSound(USoundBase* InOpen, float InVolume) { OpenSound = InOpen; OpenSoundVolume = InVolume; }

	UFUNCTION(BlueprintCallable)
	void SetCloseSound(USoundBase* InClose, float InVolume) { ClosedSound = InClose; CloseSoundVolume = InVolume; }

	UFUNCTION(BlueprintCallable)
	void SetAnimationDuration(float ShowAnimation, float HideAnimation) { ShowAnimationDuration = ShowAnimation; HideAnimationDuration = HideAnimation; }

	UFUNCTION(BlueprintCallable)
	void SetShowMouseCursor(bool bShowMouse) { bShowCursor = bShowMouse; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBackgroundBlur> BackgroundBlur;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ContainerOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> WheelOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentSlotNameText;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (DisplayName = "슬롯"))
	TArray<UZCWheelItemWidget*> WidgetSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "팝업 애니메이션"))
	TObjectPtr<UWidgetAnimation> PopUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "테스트"))
	TArray<FWheelSlot> PreSlot;

protected:
	// 상위 클래스에서 얻어야하는 변수들
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "슬롯 위젯 클래스"))
	TSubclassOf<UZCWheelItemWidget> SlotWidgetClass;

	USoundBase* OpenSound = nullptr;
	USoundBase* ClosedSound = nullptr;

	float OpenSoundVolume = 1.0f;
	float CloseSoundVolume = 1.0f;

	bool bShowCursor = true;

	float ShowAnimationDuration = 1.0f;
	float HideAnimationDuration = 0.2f;


	// 런타임 변수들
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (DisplayName = "현재 슬롯"))
	TObjectPtr<UZCWheelItemWidget> CurrentSlot;

	int32 CurrentSlotIdx = -1;

	bool bWheelOpen = false;

	//FVector2D GamepadInterpolatedAxis;

	UAudioComponent* TempCurrentClosedOpenedSound = nullptr;
};