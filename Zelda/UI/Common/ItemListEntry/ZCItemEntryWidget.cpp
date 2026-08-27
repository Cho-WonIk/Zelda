// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ItemListEntry/ZCItemEntryWidget.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "UI/Common/ItemListEntry/ZCShortcutListObject.h"

void UZCItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	MID = Material->GetDynamicMaterial();

	MID->SetScalarParameterValue(TEXT("BorderThick"), 0.0f);
}

void UZCItemEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	SetIsFocusable(true);

	UZCShortcutListObject* EntryData = Cast<UZCShortcutListObject>(ListItemObject);
	if (!EntryData) return;

	MID->SetScalarParameterValue(TEXT("BorderThick"), 0.0f);

	if (EntryData->bIsNewItem)
	{
		ShowNewItem->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		ShowNewItem->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (EntryData->Info != FString())
	{
		Info->SetText(FText::FromString(EntryData->Info));
	}

	if (EntryData->Icon.IsValid()) 
	{
		UTexture2D* LoadedTexture = EntryData->Icon.Get();
		if (LoadedTexture)
		{
			Icon->SetBrushFromTexture(LoadedTexture);
		}
	}
	else
	{
		// 비동기 로드
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(EntryData->Icon.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(
			[=, this]() { if (Icon) { Icon->SetBrushFromTexture(EntryData->Icon.Get()); } }
		));
	}
}

void UZCItemEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (!MID) return;

	MID->SetScalarParameterValue(TEXT("BorderThick"), bIsSelected ? 0.04f : 0.0f);
}
