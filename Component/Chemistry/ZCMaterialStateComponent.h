// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Gameplay/ChemistrySystem/ChemistrySystemTable.h"
#include "ZCMaterialStateComponent.generated.h"

class UZCWorldSubsystem;
struct FElementInfo;
class AZCActor;
class UZCNiagaraComponent;

USTRUCT(BlueprintType)
struct FElementState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter = "Element"))
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpreadingCount = -1;

	// 데미지 단발성 <-> 지속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "데미지 지속성"))
	bool bIsDamageOnce = false;

	// 원소 틱 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 틱 데미지"))
	float ElementTickDamage = 0.0f;

	// 원소 전파 데미지(주변에 주는 데미지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "원소 전파 데미지"))
	float ElementSpreadDamage = 0.0f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = -1.0f;

	static const FElementState Empty;

	[[nodiscard]] FORCEINLINE bool IsEmpty() const { return Duration == -1.0f && SpreadingCount == -1; }

	void Reset() { *this = Empty; }

	FElementState() : ElementTag(FGameplayTag::EmptyTag), Duration(-1.0f), SpreadingCount(-1), TimeRemaining(-1.0f) {}
	FElementState(const FGameplayTag& InElementTag, float InDuration, int32 InPropagationCount) : ElementTag(InElementTag), Duration(InDuration), SpreadingCount(InPropagationCount), TimeRemaining(InDuration) 
	{
		bIsDamageOnce = false;
		ElementTickDamage = 0.0f;
	}

	void Init(const FReactionOut& Out, int32 InSpreadCount, float SpreadDamage)
	{
		ElementTag = Out.NewElementTag;
		Duration = Out.Duration;
		bIsDamageOnce = Out.bIsDamageOnce;
		ElementTickDamage = Out.ElementTickDamage;
		
		ElementSpreadDamage = SpreadDamage;

		SpreadingCount = InSpreadCount;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCMaterialStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCMaterialStateComponent();

protected:
	virtual void InitializeComponent() override;
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FGameplayTag& GetMaterial() { return Material; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FElementState& GetCurrentElementState() { return CurrentElementState; }

	void ApplyElementExposure(const FElementInfo& NewElementInfo);
	
	void SetCachedOwner(AZCActor* Owner) { OwnerCasted  = Owner; }
	void SetUZCNiagaraComponent(UZCNiagaraComponent* Component) { VFXComponentCached = Component; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR


protected:
	void ApplyElement(const FReactionOut& ReactionResult, const int32& SpreadingCount);

	void ProcessElementSpreading();

private:
	void StartFX(bool bEnabled);
	void LoopFX(bool bEnabled);
	void EndFX(bool bEnabled);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chemistry", meta = (DisplayName = "물질", GameplayTagFilter = "Material"))
	FGameplayTag Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FElementState CurrentElementState;

	// 원소별 현재 델타값 보관
	UPROPERTY(Transient)
	TMap<FGameplayTag, float> CurrentThresholdValueMap;

	// 현재 물질 내구도
	UPROPERTY(Transient)
	float CurrentDurability;

	// 현재 물질 정보
	const FMaterialInstanceData* MaterialData;

	// 현재 물질의 상태에 적용된 원소(Ex : 불, 전기, 얼음 등등)
	const FElementInstanceData* ElementData;

private:
	class UZCWorldSubsystem* WorldSubsystem = nullptr;

	class AZCActor* OwnerCasted = nullptr;
	class UZCNiagaraComponent* VFXComponentCached = nullptr;

	FDelegateHandle WordSubsystemHandle;
};
