// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonInputTypeEnum.h"
#include "GameData/Struct/ZCWheelStruct.h"
#include "ZCDeviceEnum.h"
#include "ZCPlayerDeviceComponent.generated.h"

struct FWheelItem;
struct FWheelSlot;

class UZCWheelWidget;

class UZCUltrahandSkillComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCPlayerDeviceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCPlayerDeviceComponent();

//protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 위젯을 미리 로드 후 비활성화
	void InitWidget();

	// 스킬 선택 로직 활성화
	void StartChoseSkill();
	// 스킬 선택 로직 비활성화
	void StopChoseSkill();

	// 현재 입력모드(키보드, 게임패드)
	void SetCurrentInput(ECommonInputType NewType) { CurrentInputType = NewType; }

public:
	void UpdateInputPosition(FVector2D& InputPos);
protected:
	void CalculateMousePos(FVector2D& InputPos);
	void CalculateGamepadPos(FVector2D& InputPos);

private:
	ESkillType SkillSlotIdxToEnum(int32 Idx);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "위젯 클래스"))
	TSubclassOf<class UZCWheelWidget> WheelWidgetClass;

	// 여기서는 오른손 스킬 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "위젯 아이템"))
	TArray<FWheelSlot> SkillSlot;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "회전 편차"))
	float RotationOffset = 0.0f;

	// 휠의 정렬 방향(중앙, 왼쪽, 오른쪽)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "휠 정렬"))
	EWheelPosition WheelPos;

	// 마우스 데드 존
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "마우스 데드 존"))
	float MouseDistanceDeadZone = 100.0f;
	// 게임 패드 데드 존
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "게임패드 데드 존"))
	float GamepadStickDeadZone = 0.2f;

	// 경계
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel", meta = (DisplayName = "테두리 색상"))
	FLinearColor WheelBorderColor;

	// 배경
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Background", meta = (DisplayName = "배경 블러 강도"))
	float BackgroundBlueStrength = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Background", meta = (DisplayName = "배경 이미지"))
	TObjectPtr<UObject> WheelBackGroundImage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Background", meta = (DisplayName = "배경 이미지 색상"))
	FLinearColor WheelBackgroundImageColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Background", meta = (DisplayName = "배경 색상"))
	FLinearColor WheelBackgroundColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Background", meta = (DisplayName = "배경 이미지 사이즈"))
	FVector2D WheelBackgroundOffset;

	// 현재 선택한 위젯 정보
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|CurrentSlot", meta = (DisplayName = "색상"))
	FSlateColor WheelSlotColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|CurrentSlot", meta = (DisplayName = "텍스트 대소문자 정책"))
	ETextTransformPolicy TextTransformPolicy;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|CurrentSlot", meta = (DisplayName = "텍스트 편차"))
	FWidgetTransform WheelSelectTextOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|CurrentSlot", meta = (DisplayName = "텍스트 그림자 색상"))
	FLinearColor WheelSelectionTextShadowColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|CurrentSlot", meta = (DisplayName = "텍스트 그림자 편차"))
	FVector2D WheelSelectTextShadowOffset;

	// 마우스 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Mouse", meta = (DisplayName = "마우스 범위 락 설정"))
	uint8 LockMouseToRadius : 1 = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Mouse", meta = (DisplayName = "마우스 최소 락"))
	float MinLockMouseRedius = 300.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Mouse", meta = (DisplayName = "마우스 최대 락"))
	float MaxLockMouseRedius = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Mouse", meta = (DisplayName = "마우스 표시(키보드 사용시)"))
	bool bShowCurserWhenUsingMouse = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|Mouse", meta = (DisplayName = "마우스 표시(게임패드 사용시)"))
	bool bShowCurserWhenUsingGamepad = true;

	// 효과 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "활성화 효과음"))
	TObjectPtr<USoundBase> OpenSound = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "바활성화 효과음"))
	TObjectPtr<USoundBase> ClosedSound = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "활성화 애니메이션 지속시간"))
	float ShowAnimationDuration = 0.3f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "비활성화 애니메이션 지속시간"))
	float HideAnimationDuration = 0.15f;

	// 볼륨설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "활성화 볼륨 크기"))
	float OpenSoundVolume = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wheel|FX", meta = (DisplayName = "비활성화 볼륨 크기"))
	float CloseSoundVolume = 1.0f;

private:
	TObjectPtr<class UZCWheelWidget> WheelWidget;
	const FGeometry* WheelGeometry;
	ECommonInputType CurrentInputType;

	int32 CurrentIdx = -1;

public:
	ESkillType GetCurrentSkillType() { return CurrentSkillType; }

protected:

	ESkillType CurrentSkillType = ESkillType::Ultrahand;
};
