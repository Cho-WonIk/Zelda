// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Actor/Item/ZCItemActor.h"
#include "Struct/Enum/ZCItemType.h"
#include "ZCItemSpawnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCItemSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCItemSpawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
public:
	// 아이템 스폰 실행
	void SpawnItem();

public:
	/*====== 스폰너 설정 =====*/
	// 생성 즉시 스폰 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (DisplayName = "생성 즉시 스폰 여부"))
	bool bSpawnOnBeginPlay = true;


protected:
	/*====== 스폰할 아이템 정보 =====*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (DisplayName = "아이템 클래스"))
	TSubclassOf<class AZCItemActor> ItemClass; // 아이템 클래스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (DisplayName = "아이템 종류"))
	EItemType ItemType = EItemType::None; // 아이템 종류

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type", meta = (DisplayName = "아이템 ID"))
	int32 ItemID = 0; // 아이템 ID
};
