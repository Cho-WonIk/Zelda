// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Gameplay/ChemistrySystem/ChemistrySystemTable.h"
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
	void ConvertTableToMapRules();

public:
	//반응 규착 찾는 API
	const FReactionOut* FindReaction(const FReactionKey& key) const;

	// 모든 원소 CDO 추출
	void GetAllElementCDOs(TArray<const FElementCDO*>& Out) const;
	// 모든 물질 CDO 추출
	void GetAllMaterialCDOs(TArray<const FMaterialCDO*>& Out) const;

	const TMap<FGameplayTag, const FMaterialCDO*>& GetMaterialMap() const { return Material; }
	const TMap<FGameplayTag, const FElementCDO*>& GetElementMap() const { return Element; }

protected:
	UPROPERTY()
	UDataTable* ReactionTable = nullptr;
	UPROPERTY()
	UDataTable* MaterialTable = nullptr;
	UPROPERTY()
	UDataTable* ElementTable = nullptr;

	TMap<FReactionKey, const FReactionOut*> Rule;
	TMap<FGameplayTag, const FMaterialCDO*> Material;
	TMap<FGameplayTag, const FElementCDO*> Element;

};
