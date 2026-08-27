// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/PrimaryData/ZCPrimaryDataAsset.h"
#include "GameData/Enum/ZCWidget.h"
#include "ZCWheelSlotStylePrimaryDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWheelSlotWidgetInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "두께"))
	float Thickness = 13.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "색깔"))
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "밀도"))
	float Density = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "간격"))
	float Offset = 0.0f;
};

UCLASS()
class ZELDA_API UZCWheelSlotStylePrimaryDataAsset : public UZCPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual EZCPrimaryAssetType GetZCPrimaryAssetType() const { return EZCPrimaryAssetType::Widget; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "메인", meta = (DisplayName = "반지름"))
	float Radius = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "메인")
	FWheelSlotWidgetInfo ProceduralSetting;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "안쪽 테두리", meta = (DisplayName = "표시 여부"))
	uint8 bShowInnerBorder : 1 = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "안쪽 테두리")
	FWheelSlotWidgetInfo InnerBorderSetting;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "바깥 테두리", meta = (DisplayName = "표시 여부"))
	uint8 bShowOuterBorder : 1 = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "바깥 테두리")
	FWheelSlotWidgetInfo OuterBorderSetting;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "이미지", meta = (DisplayName = "이미지"))
	TObjectPtr<UTexture2D> Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "이미지", meta = (DisplayName = "색상"))
	FLinearColor ImageColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "이미지", meta = (DisplayName = "이미지 크기"))
	FVector2D ImageSize = FVector2D(1024.0f, 1024.0f);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "소리", meta = (DisplayName = "슬롯 선택 효과음"))
	TObjectPtr<USoundBase> SelectedSlotSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "소리", meta = (DisplayName = "슬롯 선택 효과음 크기"))
	float SelectedSlotSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "소리", meta = (DisplayName = "슬롯 선택 취소 효과음"))
	TObjectPtr<USoundBase> UnselectedSlotSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "소리", meta = (DisplayName = "슬롯 선택 취소 효과음 크기"))
	float UnselectedSlotSoundVolume = 1.0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "컨텐츠 간격"))
	float ContentOffset = 230.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "컨텐츠 간격(선택시)"))
	float SelectedContentOffset = 236.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "아이콘 색상"))
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "아이콘 색상(선택시)"))
	FLinearColor SelectedIconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "텍스트 색상(선택시)"))
	FLinearColor InfoTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "텍스트 폰트"))
	FSlateFontInfo InfoTextFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "텍스트 변환 정책"))
	ETextTransformPolicy InfoTextTransformPolicy = ETextTransformPolicy::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "텍스트 그림자 색상"))
	FLinearColor InfoTextShadowColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (DisplayName = "텍스트 그림자 간격"))
	FVector2D InfoTextShadowOffset = FVector2D(0.f, 0.f);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "슬롯 종류"))
	EWheelSlotType SlotType = EWheelSlotType::Procedural;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "여백"))
	float Padding = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "세로 간격"))
	float VerticalOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "세로 간격(선택시)"))
	float SelectedVerticalOffset = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "선택 애니메이션 지속시간"))
	float SelectedAnimationDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "미 선택 애니메이션 지속시간"))
	float UnselectedAnimationDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "일반", meta = (DisplayName = "빈 슬롯 오버레이 컬러"))
	FLinearColor EmptySlotColorOverlay = FLinearColor::Transparent;
};
