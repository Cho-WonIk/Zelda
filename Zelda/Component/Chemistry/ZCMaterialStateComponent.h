// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Physics/ZCSurface.h"
#include "GameData/Table/ChemistrySystemTable.h"
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
	// Setter
	void SetCachedOwner(AZCActor* Owner) { OwnerCasted = Owner; }
	void SetUZCNiagaraComponent(UZCNiagaraComponent* Component) { VFXComponentCached = Component; }
	void SetMaterial(FGameplayTag& InMaterial) { Material = InMaterial; }

	void NotifyTakeDamageExposure(const FElementInfo& NewElementInfo);
	void NotifyHit(const FVector& Location, const FVector& Direction);

	// Getter
	UFUNCTION(BlueprintCallable, Category = "Zelda|Chemistry")
	FORCEINLINE FGameplayTag& GetMaterial() { return Material; }

	UFUNCTION(BlueprintCallable, Category = "Zelda|Chemistry")
	bool HasActiveElement() const;

	UFUNCTION(BlueprintCallable, Category = "Zelda|Chemistry")
	FGameplayTag GetCurrentElementTag() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chemistry", meta = (DisplayName = "물질", GameplayTagFilter = "Material"))
	FGameplayTag Material;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry|Debug", meta = (DisplayName = "디버그 원소", GameplayTagFilter = "Element"))
	FGameplayTag DebugElementTag;
#endif //WITH_EDITORONLY_DATA

private:
	// Subsystem에 등록된 데이터를 가리키는 핸들
	FZCChemistryHandle ChemistryHandle;

	UPROPERTY(Transient)
	TObjectPtr<UZCWorldSubsystem> WorldSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<AZCActor> OwnerCasted;

	UPROPERTY(Transient)
	TObjectPtr<UZCNiagaraComponent> VFXComponentCached;
};
