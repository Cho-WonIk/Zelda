
#include "ZCEditorStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "EditorStyleSet.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FZCEditorStyle::Instance = nullptr;

void FZCEditorStyle::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FZCEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FZCEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ZCEditorStyle"));
	return StyleSetName;
}

const ISlateStyle& FZCEditorStyle::Get()
{
	return *Instance;
}

const FSlateBrush* FZCEditorStyle::GetBrush(FName PropertyName, const ANSICHAR* Specifier /* = nullptr */)
{
	return Instance->GetBrush(PropertyName, Specifier);
}

#define IMAGE_BRUSH(RelativePath, ...)	FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(Style->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...)	FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...)		FSlateFontInfo(Style->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...)		FSlateFontInfo(Style->RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon16x24(16.0f, 24.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon25x25(25.0f, 25.0f);
const FVector2D Icon22x28(22.0f, 28.0f);
const FVector2D Icon30x30(30.0f, 30.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon40x25(40.0f, 25.0f);
const FVector2D Icon50x50(50.0f, 50.0f);
const FVector2D Icon60x60(60.0f, 60.0f);
const FVector2D Icon120x120(120.0f, 120.0f);
const FVector2D Icon128x128(120.0f, 120.0f);

TSharedRef<FSlateStyleSet> FZCEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	FString ContentDir = FPaths::ProjectContentDir() / TEXT("ZeldaClone/Developer");
	Style->SetContentRoot(ContentDir);

	//Asset Icons
	Style->Set(TEXT("ClassThumbnail.ZCActorPrimaryDataAsset"), new IMAGE_BRUSH(TEXT("ZCActorPrimary"), Icon120x120));

	Style->Set(TEXT("ZCActorPrimaryDataAsset_PlaceholderButtonIcon"), new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH
#undef IMAGE_BRUSH_SVG
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

#undef PARENT_DIR