// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "GameData/Table/ZCChemistryTable.h"

#include "ZCChemistryGISubsystem.generated.h"

UCLASS() 
class ZELDA_API UZCChemistryGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UZCChemistryGISubsystem& Get(const UObject* Context);
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void ConvertChemistryTableToMap();

public:
	// 반응 규칙 검색
	const FZCReactionOut* FindReaction(const FZCReactionKey& Key) const;

	const TMap<FZCReactionKey, const FZCReactionOut*>&	GetRuleMap()		const { return Rule;		}
	const TMap<FGameplayTag, const FZCElementValue*>&	GetElementMap()		const { return Element;		}
	const TMap<FGameplayTag, const FZCSubstanceValue*>& GetMaterialMap()	const { return Material;	}
	const TMap<FGameplayTag, const FZCArmorValue*>&		GetArmorMap()		const { return Armor;		}
	const TMap<FGameplayTag, const FZCMonsterValue*>&	GetMonsterMap()		const { return Monster;		}

	// 원소 특성 조회
	const FZCElementValue* FindElementValue(const FGameplayTag& ElementTag) const;

	// 물질(재질) 특성 조회
	const FZCSubstanceValue* FindSubstanceValue(const FGameplayTag& MaterialTag) const;

	// 방어구 특성 조회
	const FZCArmorValue* FindArmorValue(const FGameplayTag& ArmorTag) const;

	// 몬스터 특성 조회
	const FZCMonsterValue* FindMonsterValue(const FGameplayTag& MonsterTag) const;

private:
	// 전체 반응 규칙
	TMap<FZCReactionKey, const FZCReactionOut*> Rule;
	// 원소
	TMap<FGameplayTag, const FZCElementValue*> Element;
	// 물질
	TMap<FGameplayTag, const FZCSubstanceValue*> Material;
	// 방어구
	TMap<FGameplayTag, const FZCArmorValue*> Armor;
	// 몬스터
	TMap<FGameplayTag, const FZCMonsterValue*> Monster;
};
