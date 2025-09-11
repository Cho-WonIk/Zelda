// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Subsystem/ZCChemistryGISubsystem.h"
#include "Engine/OverlapResult.h"
#include "Physics/ZCCollision.h"
#include "Development/ZCLogger.h"

UZCChemistryGISubsystem& UZCChemistryGISubsystem::Get(const UObject* Context)
{
	check(Context);
	return *Context->GetWorld()->GetGameInstance()->GetSubsystem<UZCChemistryGISubsystem>();
}

void UZCChemistryGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 데이터 테이블 로드

	ElementTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_ElementCDO.DT_ElementCDO")));
	MaterialTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_MaterialCDO.DT_MaterialCDO")));
	ReactionTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_Reaction.DT_Reaction")));

	ConvertTableToMapRules();
}

void UZCChemistryGISubsystem::Deinitialize()
{
	Rule.Reset();
	Material.Reset();
	Element.Reset();
	Super::Deinitialize();
}

void UZCChemistryGISubsystem::ConvertTableToMapRules()
{
	Rule.Reset();
	Material.Reset();
	Element.Reset();

	if (ReactionTable)
	{
		TArray<FReactionRuleRow*> Rows;
		ReactionTable->GetAllRows(TEXT("Chemistry ReactionRow"), Rows);

		for (const FReactionRuleRow* R : Rows)
		{
			const FReactionKey K{ R->SourceTag, R->TargetTag };
			Rule.Add(K, &R->Outcome);
		}
		//UZCLogger::Warning(TEXT("Reaction Table 로드 완료"));
	}

	if (MaterialTable)
	{
		TArray<FMaterialCDO*> Rows;
		MaterialTable->GetAllRows(TEXT("Chemistry Material CDO"), Rows);

		for (const FMaterialCDO* R : Rows)
		{
			Material.Add(R->Tag, R);
		}
		//UZCLogger::Warning(TEXT("Material Table 로드 완료"));

	}

	if (ElementTable)
	{
		TArray<FElementCDO*> Rows;
		ElementTable->GetAllRows(TEXT("Chemistry Element CDO"), Rows);

		for (const FElementCDO* R : Rows)
		{
			Element.Add(R->Tag, R);
			UZCLogger::Warning(TEXT("Element Table 로드 중 {0}"), R->Tag);
		}
		//UZCLogger::Warning(TEXT("Element Table 로드 완료"));
	}
}

const FReactionOut* UZCChemistryGISubsystem::FindReaction(const FReactionKey& key) const
{
	if (const FReactionOut* const* Out = Rule.Find(key))
	{
		return *Out;
	}
	return nullptr;
}

void UZCChemistryGISubsystem::GetAllElementCDOs(TArray<const FElementCDO*>& Out) const
{
	if (Element.IsEmpty()) return;
	Out.Reset();

	for (const auto &Pair : Element)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}
}

void UZCChemistryGISubsystem::GetAllMaterialCDOs(TArray<const FMaterialCDO*>& Out) const
{
	if (Material.IsEmpty()) return;

	Out.Reset();

	for (const auto &Pair : Material)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}
}
