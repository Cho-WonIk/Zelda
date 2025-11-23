// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ZCStaticActor.h"
#include "GameData/Table/ZCItemTable.h"
#include "Interface/ZCItemInterface.h"
#include "ZCItemActor.generated.h"

UCLASS(Blueprintable)
class ZELDA_API AZCItemActor : public AZCStaticActor, public IZCItemInterface
{
	GENERATED_BODY()
	
public:
	AZCItemActor(const FObjectInitializer& ObjectInitializer);

	/*===========IZCItem인터페이스===========*/
public:
	virtual int32 GetItemID() const override { return ItemInfo->ID; }
	virtual EItemType GetItemType() const override { return ItemInfo->ItemType; }
	virtual const FZCItemTable* GetItemInfo() const override { return ItemInfo; }
	virtual TUniquePtr<FZCItemTable> OnItemPickUp() override;

public:
	/*===========생성 함수===========*/
	virtual void Initialize(FZCActorTable* NewInfo) override;
	void SetItemIndex(int32 NewIndex) { ItemIndex = NewIndex; }

	/*===========오버라이드 함수들===========*/
public:
	virtual void SetOwner(AActor* NewOwner) override;

	/*===========콜리전 함수들===========*/
protected:
	virtual void OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;


	/*===========아이템 구성===========*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> SphereInteractionArea;

	/*===========아이템 정보===========*/
protected:
	struct FZCItemTable* ItemInfo = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Info")
	int32 ItemIndex = INDEX_NONE; // 아이템 인덱스(인벤토리 혹은 WorldSubsystem에서 관리하는 인덱스)
};
