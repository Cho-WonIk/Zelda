// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Character/Monster/ZCMonsterCharacter.h"
#include "ZCMonsterSpawnComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName = "ZC 몬스터 스폰"))
class ZELDA_API UZCMonsterSpawnComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCMonsterSpawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// 몬스터 스폰 실행
	void SpawnMonster();

	/*====== 스포너 설정 =====*/
public:
	// 스폰너가 월드에 생성되는 즉시 몬스터 스폰 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (DisplayName = "생성 즉시 스폰 여부"))
	bool bSpawnOnBeginPlay = true; // 생성 즉시 스폰 여부

	/*====== 스폰너 구성 =====*/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> SpawnDirection;

	/*====== 스폰할 몬스터 정보 =====*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (DisplayName = "몬스터 클래스"))
	TSubclassOf<class AZCMonsterCharacter> MonsterClass; // 몬스터 클래스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Idle", meta = (DisplayName = "Idle 시 상태"))
	EIdleType IdleType = EIdleType::None; // Idle 시 상태

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Idle", meta = (DisplayName = "Talk시 몬스터 배치", EditCondition = "IdleType == EIdleType::Talk"))
	uint8 PlaceMonsterOnTalk = 255;
		
};
