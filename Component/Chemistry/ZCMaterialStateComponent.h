// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameData/Table/ChemistrySystemTable.h"
#include "Physics/ZCSurface.h"
#include "GameData/Struct/ZCChemistryStruct.h"
#include "ZCMaterialStateComponent.generated.h"

class UZCWorldSubsystem;
class UZCNiagaraComponent;
class AZCActor;
struct FElementInfo;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCMaterialStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCMaterialStateComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Subsystem으로 전달하는 인터페이스
	UFUNCTION(BlueprintCallable)
	void ApplyElementExposure(const FElementInfo& NewElementInfo);

	void NotifyHit(const FVector& Location, const FVector& Direction);

	bool HasActiveElement() const;

	// Setter
	void SetCachedOwner(AZCActor* Owner) { OwnerCasted = Owner; }
	void SetUZCNiagaraComponent(UZCNiagaraComponent* Component) { VFXComponentCached = Component; }
	void SetMaterial(FGameplayTag& InMaterial) { Material = InMaterial; }

	// Getter
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FGameplayTag& GetMaterial() { return Material; }

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetCurrentElementTag() const;

	UFUNCTION(BlueprintCallable)
	bool IsElementActive() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chemistry", meta = (DisplayName = "물질", GameplayTagFilter = "Material"))
	FGameplayTag Material;

	// 에디터 전용 (디버깅/테스트용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry|Debug", meta = (DisplayName = "디버그 원소", GameplayTagFilter = "Element"))
	FGameplayTag DebugElementTag;

private:
	// Subsystem에 등록된 데이터를 가리키는 핸들
	FMaterialHandle SystemHandle;

	UPROPERTY(Transient)
	UZCWorldSubsystem* WorldSubsystem = nullptr;

	UPROPERTY(Transient)
	AZCActor* OwnerCasted = nullptr;

	UPROPERTY(Transient)
	UZCNiagaraComponent* VFXComponentCached = nullptr;
};
