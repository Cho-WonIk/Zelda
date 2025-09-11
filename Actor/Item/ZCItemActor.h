// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ZCActor.h"
#include "GameData/ZCItemTable.h"
#include "Interface/ZCItemInterface.h"
#include "ZCItemActor.generated.h"

UCLASS(Blueprintable)
class ZELDA_API AZCItemActor : public AZCActor, public IZCItemInterface
{
	GENERATED_BODY()
	
public:
	AZCItemActor();

	/*===========IZCItem인터페이스===========*/
public:
	virtual int32 GetID() const override { return Info->ID; }
	virtual EItemType GetType() const override { return Info->Type; }
	virtual const FZCItemTable* GetInfo() const override { return Info; }
	virtual TUniquePtr<FZCItemTable> OnItemPickUp() override;

public:
	/*===========생성 함수===========*/
	virtual void Initialize(FZCItemTable* NewItem);
	void SetItemIndex(int32 NewIndex) { ItemIndex = NewIndex; }

	/*===========오버라이드 함수들===========*/
public:
	virtual void SetOwner(AActor* NewOwner) override;

protected:
	void SetOutlineMaterial(bool bEnable);


	/*===========콜리전 함수들===========*/
protected:
	UFUNCTION()
	void OnEnterHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitHighlightRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnEnterPickUpRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitPickUpRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	/*===========아이템 구성===========*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interation, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> HighlightArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> InteractionArea;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> Mesh;


	/*===========아이템 정보===========*/
protected:
	struct FZCItemTable* Info = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Info")
	int32 ItemIndex = INDEX_NONE; // 아이템 인덱스(인벤토리 혹은 WorldSubsystem에서 관리하는 인덱스)

// VFX
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlay Material")
	TObjectPtr<class UMaterialInterface> OutlineMaterial;

	static UMaterialInterface* DefaultOutlineMaterial;

};
