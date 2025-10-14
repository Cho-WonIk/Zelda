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

	ObjectElementTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_ElementCDO.DT_ElementCDO")));
	ObjectMaterialTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_MaterialCDO.DT_MaterialCDO")));
	ObjectReactionTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_Reaction.DT_Reaction")));

	CharacterElementTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_CharacterElementCDO.DT_CharacterElementCDO")));
	CharacterTypeTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_CharacterMonsterTypeCDO.DT_CharacterMonsterTypeCDO")));
	CharacterReactionTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/ZeldaClone/Gameplay/Chemistry/DT_CharacterReaction.DT_CharacterReaction")));

	ConvertObjectTableToMapRules();
	ConvertCharacterTableToMapRules();
}

void UZCChemistryGISubsystem::Deinitialize()
{
	ObjectRule.Reset();
	ObjectMaterial.Reset();
	ObjectElement.Reset();

	CharacterElement.Reset();
	CharacterType.Reset();
	CharacterRule.Reset();

	ObjectElementTable = nullptr;
	ObjectMaterialTable = nullptr;
	ObjectReactionTable = nullptr;

	CharacterElementTable = nullptr;
	CharacterTypeTable = nullptr;
	CharacterReactionTable = nullptr;

	Super::Deinitialize();
}

void UZCChemistryGISubsystem::ConvertObjectTableToMapRules()
{
	ObjectRule.Reset();
	ObjectMaterial.Reset();
	ObjectElement.Reset();

	if (ObjectReactionTable)
	{
		TArray<FReactionRuleRow*> Rows;
		ObjectReactionTable->GetAllRows(TEXT("Chemistry ReactionRow"), Rows);

		for (const auto &R : Rows)
		{
			const FReactionKey K{ R->SourceTag, R->TargetTag };
			ObjectRule.Add(K, &R->Outcome);
		}

		//UZCLogger::Warning(TEXT("Reaction Table 로드 완료"));
	}

	if (ObjectMaterialTable)
	{
		TArray<FMaterialCDO*> Rows;
		ObjectMaterialTable->GetAllRows(TEXT("Chemistry Material CDO"), Rows);

		for (const auto &R : Rows)
		{
			ObjectMaterial.Add(R->Tag, R);
		}
		//UZCLogger::Warning(TEXT("Material Table 로드 완료"));

	}

	if (ObjectElementTable)
	{
		TArray<FElementCDO*> Rows;
		ObjectElementTable->GetAllRows(TEXT("Chemistry Element CDO"), Rows);

		for (const auto &R : Rows)
		{
			ObjectElement.Add(R->Tag, R);
		}
		//UZCLogger::Warning(TEXT("Element Table 로드 완료"));
	}
}

void UZCChemistryGISubsystem::ConvertCharacterTableToMapRules()
{
	CharacterRule.Reset();
	CharacterElement.Reset();
	CharacterType.Reset();

	if (CharacterReactionTable)
	{
		TArray<FCharacterReactionRuleRow*> Rows;
		CharacterReactionTable->GetAllRows(TEXT("Chemistry Character ReactionRow"), Rows);

		for (const auto& R : Rows)
		{
			const FCharacterReactionKey K{ R->SourceElementTag, R->CharacterTag };
			CharacterRule.Add(K, &R->Outcome);
		}
		UZCLogger::Warning(TEXT("CharacterReactionTable 로드 완료"));
	}

	if (CharacterTypeTable)
	{
		TArray<FCharacterMonsterTypeCDO*> Rows;
		CharacterTypeTable->GetAllRows(TEXT("Chemistry Character Monster CDO"), Rows);

		for (const auto &R : Rows)
		{
			CharacterType.Add(R->Tag, R);
		}
		UZCLogger::Warning(TEXT("CharacterTypeTable 로드 완료"));
	}

	if (CharacterElementTable)
	{
		TArray<FCharacterElementCDO*> Rows;
		CharacterElementTable->GetAllRows(TEXT("Chemistry Character Element CDO"), Rows);

		for (const auto &R : Rows)
		{
			CharacterElement.Add(R->Tag, R);
		}
		UZCLogger::Warning(TEXT("CharacterElementTable 로드 완료"));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FReactionOut* UZCChemistryGISubsystem::FindObjectReaction(const FReactionKey& key) const
{
	if (const FReactionOut* const* Out = ObjectRule.Find(key))
	{
		return *Out;
	}
	return nullptr;
}

void UZCChemistryGISubsystem::GetAllObjectElementCDOs(TArray<const FElementCDO*>& Out) const
{
	if (ObjectElement.IsEmpty()) return;
	Out.Reset();

	for (const auto &Pair : ObjectElement)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}
}

void UZCChemistryGISubsystem::GetAllObjectMaterialCDOs(TArray<const FMaterialCDO*>& Out) const
{
	if (ObjectMaterial.IsEmpty()) return;
	Out.Reset();

	for (const auto &Pair : ObjectMaterial)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FCharacterReactionOut* UZCChemistryGISubsystem::FindCharacterReaction(const FCharacterReactionKey& Key) const
{
	if (const FCharacterReactionOut* const* Out = CharacterRule.Find(Key))
	{
		return *Out;
	}
	return nullptr;
}

void UZCChemistryGISubsystem::GetAllCharacterElementCDOs(TArray<const FCharacterElementCDO*>& Out) const
{
	if (CharacterElement.IsEmpty()) return;
	Out.Reset();

	for (const auto& Pair : CharacterElement)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}
}

void UZCChemistryGISubsystem::GetAllCharacterTypeCDOs(TArray<const FCharacterMonsterTypeCDO*>& Out) const
{
	if (CharacterType.IsEmpty()) return;
	Out.Reset();

	for (const auto& Pair : CharacterType)
	{
		if (Pair.Value) Out.Add(Pair.Value);
	}

}
