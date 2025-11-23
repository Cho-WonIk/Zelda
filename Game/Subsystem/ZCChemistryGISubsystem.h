// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameData/Table/ChemistrySystemTable.h"
#include "GameData/Table/ChemistrySystemCharacterTable.h"
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
	void ConvertObjectTableToMapRules();
	void ConvertCharacterTableToMapRules();

public:
	//반응 규착 찾는 API, 물체용
	const FReactionOut* FindObjectReaction(const FReactionKey& key) const;

	// 모든 원소 CDO 추출
	void GetAllObjectElementCDOs(TArray<const FElementCDO*>& Out) const;
	// 모든 물질 CDO 추출
	void GetAllObjectMaterialCDOs(TArray<const FMaterialCDO*>& Out) const;

	const TMap<FGameplayTag, const FMaterialCDO*>& GetObjectMaterialMap() const { return ObjectMaterial; }
	const TMap<FGameplayTag, const FElementCDO*>& GetObjectElementMap() const { return ObjectElement; }

public:
	//반응 규칙 찾는 API, 캐릭터용
	const FCharacterReactionOut* FindCharacterReaction(const FCharacterReactionKey& Key) const;

	// 모든 캐릭터에 적용되는 원소 CDO 추출
	void GetAllCharacterElementCDOs(TArray<const FCharacterElementCDO*>& Out) const;
	// 모든 몬스터 특성 추출
	void GetAllCharacterTypeCDOs(TArray<const FCharacterMonsterTypeCDO*>& Out) const;

	const TMap<FGameplayTag, const FCharacterElementCDO*>& GetCharacterElementMap() const { return CharacterElement; }
	const TMap<FGameplayTag, const FCharacterMonsterTypeCDO*>& GetCharacterTypeMap() const { return CharacterType; }


protected:
	TMap<FReactionKey, const FReactionOut*> ObjectRule;
	TMap<FGameplayTag, const FMaterialCDO*> ObjectMaterial;
	TMap<FGameplayTag, const FElementCDO*> ObjectElement;

protected:
	TMap< FCharacterReactionKey, const FCharacterReactionOut*> CharacterRule;
	TMap<FGameplayTag, const FCharacterElementCDO*> CharacterElement;
	TMap<FGameplayTag, const FCharacterMonsterTypeCDO*> CharacterType;
};
