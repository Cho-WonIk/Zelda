// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ZCGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZELDA_API UZCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR


public:
	/*=========================== 아이템 테이블 ===============================*/
	const UDataTable* const GetNoramlItemTable()			{ return NoramlItemTable;			}
	const UDataTable* const GetWeaponItemTable()			{ return WeaponItemTable;			}
	const UDataTable* const GetShieldItemTable()			{ return ShieldItemTable;			}

	/*=========================== 화학 테이블 ===============================*/
	const UDataTable* const GetChemistryElementTable()		{ return ChemistryElementTable;		}
	const UDataTable* const GetChemistryMaterialTable()		{ return ChemistryMaterialTable;	}

	const UDataTable* const GetChemistryMonsterTable()		{ return ChemistryMonsterTable;		}
	const UDataTable* const GetChemistryArmorTable()		{ return ChemistryArmorTable;		}
	const UDataTable* const GetChemistryReactionTable()		{ return ChemistryReactionTable;	}

	/*========================= 화학 엔진 전체 테이블 =========================*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UDataTable> ChemistryElementTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UDataTable> ChemistryMaterialTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UDataTable> ChemistryMonsterTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UDataTable> ChemistryArmorTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UDataTable> ChemistryReactionTable = nullptr;

	/*=========================== 아이템 테이블 ===============================*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> NoramlItemTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> WeaponItemTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> ShieldItemTable = nullptr;
};
