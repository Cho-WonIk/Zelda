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
	/*=========================== 화학 엔진 오브젝트 테이블 ===============================*/
	const UDataTable* const GetObjectReactionTable()		{ return ObjectReactionTable;		}
	const UDataTable* const GetObjectMaterialTable()		{ return ObjectMaterialTable;		}
	const UDataTable* const GetObjectElementTable()			{ return ObjectElementTable;		}

	/*=========================== 화학 엔진 캐릭터 테이블 ===============================*/
	const UDataTable* const GetCharacterReactionTable()		{ return CharacterReactionTable;	}
	const UDataTable* const GetCharacterElementTable()		{ return CharacterElementTable;		}
	const UDataTable* const GetCharacterTypeTable()			{ return CharacterTypeTable;		}

	/*=========================== 아이템 테이블 ===============================*/
	const UDataTable* const GetNoramlItemTable()			{ return NoramlItemTable;			}
	const UDataTable* const GetWeaponItemTable()			{ return WeaponItemTable;			}
	const UDataTable* const GetShieldItemTable()			{ return ShieldItemTable;			}

protected:
	/*=========================== 화학 엔진 오브젝트 테이블 ===============================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Object")
	TObjectPtr<UDataTable> ObjectReactionTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Object")
	TObjectPtr<UDataTable> ObjectMaterialTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Object")
	TObjectPtr<UDataTable> ObjectElementTable = nullptr;

	/*=========================== 화학 엔진 캐릭터 테이블 ===============================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Character")
	TObjectPtr<UDataTable> CharacterReactionTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Character")
	TObjectPtr<UDataTable> CharacterElementTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry | Character")
	TObjectPtr<UDataTable> CharacterTypeTable = nullptr;

	/*=========================== 아이템 테이블 ===============================*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> NoramlItemTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> WeaponItemTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UDataTable> ShieldItemTable = nullptr;
};
