// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Subsystem/ZCChemistryGISubsystem.h"
#include "Engine/OverlapResult.h"
#include "Physics/ZCCollision.h"
#include "Development/ZCLogger.h"

#include "Game/ZCGameInstance.h"

UZCChemistryGISubsystem& UZCChemistryGISubsystem::Get(const UObject* Context)
{
	check(Context);
	return *Context->GetWorld()->GetGameInstance()->GetSubsystem<UZCChemistryGISubsystem>();
}

void UZCChemistryGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConvertChemistryTableToMap();
}

void UZCChemistryGISubsystem::Deinitialize()
{
	Rule.Reset();
	Element.Reset();
	Material.Reset();
	Armor.Reset();
	Monster.Reset();

	Super::Deinitialize();
}

void UZCChemistryGISubsystem::ConvertChemistryTableToMap()
{
	Rule.Reset();
	Element.Reset();
	Material.Reset();
	Armor.Reset();
	Monster.Reset();

	UZCGameInstance* ZCGameInstance = Cast<UZCGameInstance>(GetGameInstance());

	// 원소(Element) 테이블 변환
	if (const UDataTable* ElementTable = ZCGameInstance->GetChemistryElementTable())
	{
		TArray<FZCElementCDO*> Rows;
		ElementTable->GetAllRows(TEXT("Chemistry Element CDO"), Rows);
		for (auto* Row : Rows)
		{
			if (Row && Row->Tag.IsValid())
			{
				Element.Add(Row->Tag, &Row->Value);
			}
		}
		UZCLogger::Warning(TEXT("Chemistry Element Table 로드 완료"));
	}

	// 물질(Material) 테이블 변환
	if (const UDataTable* MaterialTable = ZCGameInstance->GetChemistryMaterialTable())
	{
		TArray<FZCMaterialCDO*> Rows;
		MaterialTable->GetAllRows(TEXT("Chemistry Material CDO"), Rows);
		for (auto* Row : Rows)
		{
			if (Row && Row->Tag.IsValid())
			{
				Material.Add(Row->Tag, &Row->Value);
			}
		}
		UZCLogger::Warning(TEXT("Chemistry Material Table 로드 완료"));
	}

	// 몬스터(Monster) 특성 테이블 변환
	if (const UDataTable* MonsterTable = ZCGameInstance->GetChemistryMonsterTable())
	{
		TArray<FZCMonsterChemistryCDO*> Rows;
		MonsterTable->GetAllRows(TEXT("Chemistry Monster CDO"), Rows);
		for (auto* Row : Rows)
		{
			if (Row && Row->Tag.IsValid())
			{
				Monster.Add(Row->Tag, &Row->Value);
			}
		}
		UZCLogger::Warning(TEXT("Chemistry Monster Table 로드 완료"));
	}

	// 방어구(Armor) 특성 테이블 변환
	if (const UDataTable* ArmorTable = ZCGameInstance->GetChemistryArmorTable())
	{
		TArray<FZCArmorChemistryCDO*> Rows;
		ArmorTable->GetAllRows(TEXT("Chemistry Armor CDO"), Rows);
		for (auto* Row : Rows)
		{
			if (Row && Row->Tag.IsValid())
			{
				Armor.Add(Row->Tag, &Row->Value);
			}
		}
		UZCLogger::Warning(TEXT("Chemistry Armor Table 로드 완료"));
	}

	// 화학 반응(Reaction) 규칙 테이블 변환
	if (const UDataTable* ReactionTable = ZCGameInstance->GetChemistryReactionTable())
	{
		TArray<FZCReactionRule*> Rows;
		ReactionTable->GetAllRows(TEXT("Chemistry Reaction Rule"), Rows);
		for (auto* Row : Rows)
		{
			if (Row)
			{
				FZCReactionKey Key{ Row->SourceTag, Row->TargetTag };
				Rule.Add(Key, &Row->Outcome);
			}
		}
		UZCLogger::Warning(TEXT("Chemistry Reaction Table 로드 완료"));
	}
}

const FZCReactionOut* UZCChemistryGISubsystem::FindReaction(const FZCReactionKey& Key) const
{
	if (const FZCReactionOut* const* Out = Rule.Find(Key))
	{
		return *Out;
	}
	return nullptr;
}

const FZCElementValue* UZCChemistryGISubsystem::FindElementValue(const FGameplayTag& ElementTag) const
{
	if (const FZCElementValue* const* FoundPtr = Element.Find(ElementTag))
	{
		return *FoundPtr;
	}
	return nullptr;
}

const FZCSubstanceValue* UZCChemistryGISubsystem::FindSubstanceValue(const FGameplayTag& MaterialTag) const
{
	if (const FZCSubstanceValue* const* FoundPtr = Material.Find(MaterialTag))
	{
		return *FoundPtr;
	}
	return nullptr;
}

const FZCArmorValue* UZCChemistryGISubsystem::FindArmorValue(const FGameplayTag& ArmorTag) const
{
	if (const FZCArmorValue* const* FoundPtr = Armor.Find(ArmorTag))
	{
		return *FoundPtr;
	}
	return nullptr;
}

const FZCMonsterValue* UZCChemistryGISubsystem::FindMonsterValue(const FGameplayTag& MonsterTag) const
{
	if (const FZCMonsterValue* const* FoundPtr = Monster.Find(MonsterTag))
	{
		return *FoundPtr;
	}
	return nullptr;
}
