// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/State/ZCStateComponent.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Development/ZCDebug.h"
#include "Component/Reaction/Struct/ZCReactionEnum.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Character/ZCCharacter.h"

const FCharacterElementState FCharacterElementState::Empty = FCharacterElementState(FGameplayTag::EmptyTag, -1.0f, -1);

#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::State
{
	static bool bDrawDebugAll = false;
	static bool bDrawDebugShow = false;

	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::State::all, bDrawDebugAll, TEXT("State 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugShow(Zelda::Debug::State::show, bDrawDebugShow, TEXT("State 디버깅 상태 표시 On/Off"), ECVF_Default);
}
#endif

UZCStateComponent::UZCStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.125f;

	bWantsInitializeComponent = true;

	Health.Set(100.0f);
}

void UZCStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZCStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnHealthChanged.Clear();
	OnHealthZero.Clear();
	OnStaggerFull.Clear();

	Super::EndPlay(EndPlayReason);

}

void UZCStateComponent::InitializeComponent()
{
	Super::InitializeComponent();
	WorldSubsystem = GetWorld()->GetSubsystem<UZCWorldSubsystem>();
}

void UZCStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// CC기 적용시 틱 적용
	if (!Stagger.CCStagger.IsEmpty())
	{
		if (Stagger.CCStagger.Duration > 0.0f)
		{
			Stagger.CCStagger.Duration = FMath::Max(0.0f, Stagger.CCStagger.Duration - DeltaTime);

			if (Stagger.CCStagger.Duration == 0.0f)
			{
				Stagger.CCStagger.Reset();
				CurrentThreshold.Reset();
				ElementData = nullptr;

				CCElementFX(false);

				SetComponentTickEnabled(false);
			}
		}
		// CurrentEelementStat의 duration이 -1인 경우 무한이 지속

		// TODO : 액터 본인에게 데미지 주입 로직 작성(bIsDamageOnce가 false인 경우)

		// 주변에 전파
		ProcessElementSpreading();
	}

#if !UE_BUILD_SHIPPING
	if (Zelda::Debug::State::bDrawDebugAll || Zelda::Debug::State::bDrawDebugShow)
	{
		FVector PawnLocation = GetOwner()->GetActorLocation();
		DrawDebugString(GetWorld(), PawnLocation + FVector(0, 0, 150),
			FString::Printf(TEXT("현재 체력: %.2f / %.2f"), Health.Current, Health.Max),
			nullptr, FColor::Green, 0.0f, true);
	}
#endif // !UE_BUILD_SHIPPING
}

float UZCStateComponent::ApplyDamage(float DamageAmount, const FElementInfo& NewElementInfo, bool IsCritical)
{
	if (Health.IsZero()) return 0.0f;

	// 속성 데미지 계산은 오버라이드하여 구현
	// 데미지 = (베이스 데미지 - 방어력) x 크리티컬시(1.5배) + 속성 데미지(약점 속성이면 1.5배, 강점 속성이면 0.5)
	float AmountDamage = CalculDamage(DamageAmount, NewElementInfo.ElementTag, IsCritical);

	ApplyStagger(AmountDamage * 0.125f, NewElementInfo);

	Health.Sub(AmountDamage);
	OnHealthChanged.Broadcast(Health);

	if (Health.IsZero()) { OnHealthZero.Broadcast(); }

	//UZCLogger::Warning(TEXT("AmountDamage : {0}, StaggerAmount : {1}"), DamageAmount, StaggerAmount);

	return DamageAmount;
}

void UZCStateComponent::ApplyStagger(float AddStagger, const FElementInfo& NewElementInfo)
{
	// 피격 그로기 수치
	Stagger.DamageStagger.Add(AddStagger);

	// 원소 임계점을 넘으면 해당 원소 CC기 적용

	if (NewElementInfo.SpreadCount < 0) return;

	UZCLogger::Warning(TEXT("1"));

	FCharacterReactionOut Out;
	FCharacterElementState &CurrentCCStagger = Stagger.CCStagger;

	// 이미 원소가 적용되어 있는 경우
	if (!Stagger.CCStagger.IsEmpty())
	{
		UZCLogger::Warning(TEXT("2 - 1"));
		// 이미 적용된 원소와 새로운 원소가 동일한 원소이고
		// 현재 적용된 원소값이 먼저(먼저 전파된 경우)
		// 역전파를 막음
		if (Stagger.CCStagger.ElementTag == NewElementInfo.ElementTag && CurrentCCStagger.SpreadingCount > NewElementInfo.SpreadCount) return;

		if (!WorldSubsystem->TryGetCharacterOutcome(NewElementInfo.ElementTag, CurrentCCStagger.ElementTag, Out)) return;

		// 상쇄(EmptyTag), 적용된 원소 제거
		if (Out.Tag == FGameplayTag::EmptyTag)
		{
			CurrentCCStagger.Reset();
			CurrentThreshold.Reset();
			ElementData = nullptr;

			CCElementFX(false);
			SetComponentTickEnabled(false);
		}
	}
	// 적용된 원소가 없는 경우
	else
	{
		if (!WorldSubsystem->TryGetCharacterOutcome(NewElementInfo.ElementTag, CharacterTag, Out)) return;

		if (Out.Tag == FGameplayTag::EmptyTag) return;

		const float Threshold = Armor.Threshold.FindRef(Out.Tag);
		if (Threshold <= 0.0f) return;

		float& CurrentValue = CurrentThreshold.FindOrAdd(Out.Tag);
		CurrentValue += Armor.ThresholdDelta.FindRef(Out.Tag);

		// 임계치 미달
		if (CurrentValue < Threshold) return;
	}

	ApplyCCElement(Out, NewElementInfo.SpreadCount - 1);
	CurrentThreshold.Reset();

	return;
}

float UZCStateComponent::CalculDamage(float DamageAmount, const FGameplayTag& ElementTag, bool IsCritical)
{
	float AmountDamage = DamageAmount - Armor.ArmorState;

	if (!Armor.ImmutTag.Contains(ElementTag))
	{
		if (Armor.StrongTag.Contains(ElementTag)) AmountDamage /= 2.0f;

		if (Armor.WeakTag.Contains(ElementTag)) AmountDamage *= 1.5f;
	}

	if (IsCritical) AmountDamage *= 1.5f;
	return AmountDamage;
}

void UZCStateComponent::ApplyCCElement(const FCharacterReactionOut& ReactionResult, const int32& SpreadingCount)
{
	ElementData = WorldSubsystem->GetCharacterElementInstanceData(ReactionResult.Tag);
	if (!ElementData) return;

	// 캐릭터에 원소 속성이 적용되면
	// SpreadCount에 상관없이 퍼져나갈 수 있음(초기화됨)

	// 물체 -> 캐릭터 -> 물체로 전파되는 경우
	// 물체(전이자) -> 캐릭터 -> 물체(전이자)의 로직도 고려해야함

	// TODO : 마찰력 및 각종 CC기는 추후 구현

	Stagger.CCStagger.InitFromOut(ReactionResult, SpreadingCount);

	if (Stagger.CCStagger.bIsDamageOnce)
	{
		// TODO: 초기 데미지가 있는 경우 Owner 액터에 데미지 주입
		ReactionResult.bIsDamageOnce;
	}

	// 기존 FX 비활성화
	CCElementFX(false);

	// 새롭게 적용
	CCElementFX(true);

	SetComponentTickEnabled(true);
}

void UZCStateComponent::ProcessElementSpreading()
{
	if (!WorldSubsystem || Stagger.CCStagger.IsEmpty()) return;
	if (!ElementData) return;

	AActor* Owner = GetOwner();

	if (!Owner) return;

	// 범위 내 액터들 엘리먼트 임계치 증가 시킴, 데미지가 0.0f이어도 확산이 일어나야 함.
	FCharacterElementState& CurrentCCStagger = Stagger.CCStagger;

	FElementInfo NewInfo(CurrentCCStagger.ElementTag, CurrentCCStagger.Duration, CurrentCCStagger.SpreadingCount);
	float ApplyDamage = CurrentCCStagger.ElementTickDamage;
	UZCGameplayFunctionLibrary::ApplyRadialDamage(NewInfo, this, ApplyDamage, Owner->GetActorLocation(), ElementData->SpreadingRange, UZCDamageType::StaticClass(), { Owner }, Owner, Owner->GetInstigatorController(), false, Zelda::Channel::Damage);

}

void UZCStateComponent::CCElementFX(bool bEnabled)
{
	if (!VFXComponent) return;

	// TODO : UCurve2D에 따라 적용되는 로직을 만들어야 함

	if (bEnabled && ElementData)
	{
		if (ElementData->LoopVFX)
		{
			VFXComponent->SetAsset(ElementData->LoopVFX);
			VFXComponent->Activate(true);
		}
		if (ElementData->LoopMaterial)
		{
			OwnerCharacter->GetMesh()->SetOverlayMaterial(ElementData->LoopMaterial);
		}
	}
	else
	{
		VFXComponent->Deactivate();
		OwnerCharacter->GetMesh()->SetOverlayMaterial(nullptr);
	}
}
