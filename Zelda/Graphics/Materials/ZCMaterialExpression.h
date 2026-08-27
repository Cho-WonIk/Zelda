// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "ZCMaterialExpression.generated.h"

// UI ShortCut
UCLASS(collapsecategories, hidecategories = Object)
class ZELDA_API UZCShortcutMaterialExpression : public UMaterialExpression
{
	GENERATED_BODY()
	
public:
    UZCShortcutMaterialExpression(const FObjectInitializer& ObjectInitializer);

    // 배경 색상
    UPROPERTY()
    FExpressionInput InputColor;

    // 테두리 색상
    UPROPERTY()
    FExpressionInput BorderColor;

    // 테두리 두께, 0.0f일 경우 테두리 없음
    UPROPERTY()
    FExpressionInput BorderThick;

    virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
    virtual void GetCaption(TArray<FString>& OutCaptions) const override;

#if WITH_EDITOR
    // 검색 키워드
    //virtual FText GetKeywords() const override { return FText::FromString(TEXT("UZCMaterialExpression")); }

#endif // WITH_EDITOR
   
};
