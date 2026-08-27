// Fill out your copyright notice in the Description page of Project Settings.


#include "Graphics/Materials/ZCMaterialExpression.h"
#include "MaterialCompiler.h"

#define ZC_Category TEXT("Zelda")

UZCShortcutMaterialExpression::UZCShortcutMaterialExpression(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	MenuCategories.Add(FText::FromString(ZC_Category));
#endif // WITH_EDITOR

}

int32 UZCShortcutMaterialExpression::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	// 기본값 
	const int32 DefaultBgColor = Compiler->Constant3(1.0f, 1.0f, 1.0f);
	const int32 DefaultBorderColor = Compiler->Constant3(0.0f, 0.0f, 0.0f);
	const int32 DefaultBorderThick = Compiler->Constant(0.01f);

	const int32 BgColorCode = InputColor.Expression ? InputColor.Compile(Compiler) : DefaultBgColor;
	const int32 BorderColorCode = BorderColor.Expression ? BorderColor.Compile(Compiler) : DefaultBorderColor;
	const int32 BorderThickCode = BorderThick.Expression ? BorderThick.Compile(Compiler) : DefaultBorderThick;

	const int32 UV = Compiler->TextureCoordinate(0, false, false);
	const int32 U = Compiler->ComponentMask(UV, true, false, false, false);
	const int32 V = Compiler->ComponentMask(UV, false, true, false, false);

	const int32 OneMinusU = Compiler->Sub(Compiler->Constant(1.0f), U);
	const int32 OneMinusV = Compiler->Sub(Compiler->Constant(1.0f), V);

	const int32 MinUV = Compiler->Min(U, V);
	const int32 MinOneUV = Compiler->Min(OneMinusU, OneMinusV);
	const int32 DistToEdge = Compiler->Min(MinUV, MinOneUV); // 0(가장자리) ~ 0.5(중앙)

	const int32 InnerMask = Compiler->If(
		DistToEdge,					// A
		BorderThickCode,			// B
		Compiler->Constant(1.0f),	// A > B → 내부(배경)
		Compiler->Constant(1.0f),	// A == B → 내부(배경) 취급
		Compiler->Constant(0.0f),	// A < B → 테두리
		Compiler->Constant(0.0f)	// Threshold
		);

	const int32 BorderMask = Compiler->Sub(Compiler->Constant(1.0f), InnerMask);
	const int32 BorderPart = Compiler->Mul(BorderColorCode, BorderMask);
	const int32 InnerPart = Compiler->Mul(BgColorCode, InnerMask);
	const int32 FinalColor = Compiler->Add(BorderPart, InnerPart);

    return FinalColor;
}

void UZCShortcutMaterialExpression::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("ZC Shortcut"));
}
