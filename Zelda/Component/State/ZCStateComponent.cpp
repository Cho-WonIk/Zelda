// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/State/ZCStateComponent.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Component/Reaction/Struct/ZCReactionEnum.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "Character/ZCCharacter.h"
#include "Gameplay/Damage/ZCDamage.h"
#include "Physics/ZCShape.h"
#include "Game/Subsystem/ZCChemistryGISubsystem.h"
#include "Settings/World/ZCWorldSettings.h"
#include "Development/ZCDebug.h"
#if !UE_BUILD_SHIPPING
namespace Zelda::Debug::State
{
	static bool bDrawDebugAll = false;
	static bool bDrawDebugShow = false;

	static FAutoConsoleVariableRef CVar_DebugAll(Zelda::Debug::State::all, bDrawDebugAll, TEXT("State 디버깅 전체 On/Off"), ECVF_Default);
	static FAutoConsoleVariableRef CVar_DebugShow(Zelda::Debug::State::show, bDrawDebugShow, TEXT("State 디버깅 상태 표시 On/Off"), ECVF_Default);
}
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCStateComponent)

UZCStateComponent::UZCStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	//PrimaryComponentTick.TickInterval = 0.125f;

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
	SpreadInterval = GetDefault<UZCWorldSettings>()->SpreadInterval;
	SetArmor();
}

void UZCStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FZCCharacterElementStat& ElementStat = Stagger.ElementStagger;
	if (ElementStat.IsDeactive())
	{
		CCElementFX(false);
		SetComponentTickEnabled(false);
		return;
	}

	ElementStat.TimeRemaining += DeltaTime;

	// 지속 시간 처리(Duration이 -1.0f면 무한 지속)
	if (ElementStat.Duration > 0.0f)
	{
		ElementStat.Duration -= DeltaTime;
		if (ElementStat.Duration <= 0.0f)
		{
			ElementStat.Reset();
			CCElementFX(false);
			SetComponentTickEnabled(false);
			return;
		}
	}

	// 틱 데미지 처리
	if (ElementStat.bIsDamageOnce && ElementStat.TickDamage > 0.0f)
	{
		// 틱 간격이 0이면 매 프레임, 아니면 타이머 체크
		bool bApplyTick = false;
		if (ElementStat.TickInterval <= 0.0f)
		{
			bApplyTick = true;
		}
		else
		{
			ElementStat.DamageTimer -= DeltaTime;
			if (ElementStat.DamageTimer <= 0.0f)
			{
				bApplyTick = true;
				ElementStat.DamageTimer = ElementStat.TickInterval; // 타이머 리셋
			}
		}

		if (bApplyTick)
		{
			Health.Sub(ElementStat.TickDamage);
			OnHealthChanged.Broadcast(Health);
			if (Health.IsZero()) { OnHealthZero.Broadcast(); }
		}
	}

	// 원소 전파 (확산)
	ProcessElementSpreading(DeltaTime);

	
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

void UZCStateComponent::NotifyHit(FVector& Location, FVector& Direction)
{
	Stagger.ElementStagger.SurfaceHitArray.Add({ Location, Direction });
}

float UZCStateComponent::ApplyDamage(float DamageAmount, const FElementInfo& NewElementInfo, bool IsCritical)
{
	if (Health.IsZero()) return 0.0f;

	// 속성 데미지 계산은 오버라이드하여 구현
	// 데미지 = (베이스 데미지 - 방어력) x 크리티컬시(1.5배) + 속성 데미지(약점 속성이면 1.5배, 강점 속성이면 0.5)
	float AmountDamage = CalculDamage(DamageAmount, NewElementInfo.ElementTag, IsCritical);

	// 물리/원소 그로기(상태이상) 수치 누적
	// TODO : 임시로 설정한 0.125f
	ApplyStagger(AmountDamage * 0.125f, NewElementInfo);

	Health.Sub(AmountDamage);
	OnHealthChanged.Broadcast(Health);
	if (Health.IsZero()) { OnHealthZero.Broadcast(); }

	//UZCLogger::Warning(TEXT("AmountDamage : {0}, StaggerAmount : {1}"), DamageAmount, StaggerAmount);

	return AmountDamage;
}

void UZCStateComponent::ApplyStagger(float AddStagger, const FElementInfo& NewElementInfo)
{
	// 1. 물리적 피격 그로기 누적
	Stagger.DamageStagger.Add(AddStagger);

	if (Stagger.DamageStagger.IsFull())
	{
		OnStaggerFull.Broadcast();
	}

	// 원소(화학) 반응 처리
	// 확산 횟수가 다 되었으면 반응 처리 안함
	if (NewElementInfo.SpreadCount < 0) return;
	if (!WorldSubsystem) return;

	FZCReactionOut ReactionResult;
	FZCCharacterElementStat& EStat = Stagger.ElementStagger;

	// 이미 활성화된 상태가 있음 (원소 vs 원소)
	if (EStat.IsActive())
	{
		// 동일 원소 재적용 (지속시간 갱신 등)
		if (EStat.CurrentElementTag == NewElementInfo.ElementTag)
		{
			// 역전파 방지
			if (EStat.SpreadCount > NewElementInfo.SpreadCount) return;

			// 지속 시간 갱신
			if (EStat.Duration >= 0.0f && NewElementInfo.Duration >= 0.0f)
			{
				EStat.Duration = NewElementInfo.Duration;
			}
			return;
		}

		// 원소와의 반응 확인 (예: 불 + 물 -> 증발)
		if (WorldSubsystem->TryGetOutCome(NewElementInfo.ElementTag, EStat.CurrentElementTag, ReactionResult))
		{
			// 상쇄 반응 (결과 태그가 Empty)
			if (ReactionResult.ReactionElementTag == FGameplayTag::EmptyTag)
			{
				EStat.Reset();
				CCElementFX(false);
				SetComponentTickEnabled(false);
				return;
			}
			// 새로운 원소로 대체 또는 변환
			else
			{
				ApplyCCElement(ReactionResult, NewElementInfo.SpreadCount - 1);
			}
		}
	}
	// 활성 상태 없음 (원소 vs 캐릭터/방어구)
	else
	{
		// 캐릭터 자체 속성(CharacterTag) 또는 방어구 타입과 반응하는지 확인
		if (WorldSubsystem->TryGetOutCome(NewElementInfo.ElementTag, CharacterTag, ReactionResult))
		{
			if (ReactionResult.ReactionElementTag == FGameplayTag::EmptyTag) return;

			// 임계치(Threshold) 체크
			if (Armor.Threshold)
			{
				const float MaxThreshold = Armor.Threshold->FindRef(ReactionResult.ReactionElementTag);

				// 임계치가 설정된 속성인 경우만 누적 로직 수행
				if (MaxThreshold > 0.0f)
				{
					float& CurrentAcc = EStat.AccumulatedThresholds.FindOrAdd(ReactionResult.ReactionElementTag);
					// 들어온 원소의 강도만큼 누적
					CurrentAcc += SpreadInterval;

					if (CurrentAcc < MaxThreshold) return; // 임계치 미달
				}
			}

			// 3. 임계치 달성 혹은 임계치 없는 반응 -> 상태 적용
			ApplyCCElement(ReactionResult, NewElementInfo.SpreadCount - 1);

			// 누적치 초기화
			EStat.AccumulatedThresholds.Reset();
		}
	}
}

float UZCStateComponent::CalculDamage(float DamageAmount, const FGameplayTag& ElementTag, bool IsCritical)
{
	float FinalDamage = FMath::Max(0.0f, DamageAmount - Armor.ArmorState);

	// 속성 상성 적용 (면역 -> 강점 -> 약점 순)
	if (IsImmunt(ElementTag))
	{
		FinalDamage = 0.0f;
	}
	else if (IsStrong(ElementTag))
	{
		FinalDamage *= 0.5f; // 강점 속성이면 반감
	}
	else if (IsWeak(ElementTag))
	{
		FinalDamage *= 1.5f; // 약점 속성이면 1.5배
	}

	// 치명타 적용
	if (IsCritical)
	{
		FinalDamage *= 1.5f;
	}

	return FinalDamage;
}

void UZCStateComponent::ApplyCCElement(const FZCReactionOut& ReactionResult, const int32& SpreadCount)
{
	FZCCharacterElementStat& EStat = Stagger.ElementStagger;

	// 상태 갱신
	EStat.CurrentElementTag = ReactionResult.ReactionElementTag;
	EStat.Duration = ReactionResult.Duration;
	EStat.SpreadCount = SpreadCount;
	EStat.bIsDamageOnce = ReactionResult.bIsDamageOnce;
	EStat.TickInterval = ReactionResult.TickInterval;
	EStat.TickDamage = ReactionResult.TickDamage;

	EStat.TimeRemaining = 0.0f;

	// 틱 데미지 타이머 초기화
	EStat.DamageTimer = (!EStat.bIsDamageOnce && EStat.TickInterval > 0.0f) ? EStat.TickInterval : 0.0f;

	// 확산 타이머 초기화
	EStat.SpreadTimer = 0.1f;

	// 초기 데미지 적용
	if (ReactionResult.FirstDamage > 0.0f)
	{
		Health.Sub(ReactionResult.FirstDamage);
		OnHealthChanged.Broadcast(Health);
		if (Health.IsZero()) { OnHealthZero.Broadcast(); }
	}

	// FX 재생 및 틱 활성화
	CCElementFX(true);
	SetComponentTickEnabled(true);
}

void UZCStateComponent::ProcessElementSpreading(float DeltaTime)
{
	FZCCharacterElementStat& EStat = Stagger.ElementStagger;

	// 쿨타임 체크
	EStat.SpreadTimer -= DeltaTime;
	if (EStat.SpreadTimer > 0.0f) return;

	// 리셋 (예: 0.2초마다 확산 시도)
	EStat.SpreadTimer = 0.2f;

	if (EStat.SpreadCount <= 0) return;
	if (!OwnerCharacter || !WorldSubsystem) return;

	// 현재 적용된 원소의 상세 정보(SpreadShape 등)를 가져오기 위해 GIS나 WorldSubsystem 조회
	auto* GIS = GetWorld()->GetGameInstance()->GetSubsystem<UZCChemistryGISubsystem>();
	if (!GIS) return;

	const FZCElementValue* ElementValue = GIS->GetElementMap().FindRef(EStat.CurrentElementTag);
	if (!ElementValue) return;

	FElementInfo SpreadInfo(EStat.CurrentElementTag, EStat.Duration, EStat.SpreadCount);

	// Element 타입 확산 (범위 공격 등)
	if (EnumHasAnyFlags(static_cast<ESpreadShapeType>(ElementValue->SpreadType), ESpreadShapeType::Element))
	{
		UZCGameplayFunctionLibrary::ApplyShapeDamage(
			SpreadInfo,
			this,
			0.0f, // TODO : 확산 데미지는 0일 수도 있고 설정에 따름
			OwnerCharacter->GetMesh()->GetComponentTransform(),
			ElementValue->SpreadShape,
			UZCDamageType::StaticClass(),
			{ OwnerCharacter },
			OwnerCharacter,
			OwnerCharacter->GetInstigatorController(),
			Zelda::Channel::Damage
		);
	}

	// Object 타입 확산 (접촉한 표면으로 전파)
	if (EnumHasAllFlags(static_cast<ESpreadShapeType>(ElementValue->SpreadType), ESpreadShapeType::Object))
	{
		if (EStat.SurfaceHitArray.Num() == 0) return;

		const FTransform& CompTransform = OwnerCharacter->GetMesh()->GetComponentTransform();

		// 역순 순회하며 처리 후 제거
		for (int32 i = EStat.SurfaceHitArray.Num() - 1; i >= 0; --i)
		{
			FZCSurfaceInfo& Info = EStat.SurfaceHitArray[i];

			FVector Start = CompTransform.TransformPosition(Info.LocalLocation);
			FVector Dir = CompTransform.TransformVectorNoScale(Info.LocalDirection).GetSafeNormal();

			bool bHit = UZCGameplayFunctionLibrary::ApplyTouchDamage(
				SpreadInfo,
				this,
				0.0f,
				Start,
				Dir,
				UZCDamageType::StaticClass(),
				{ OwnerCharacter },
				OwnerCharacter,
				OwnerCharacter->GetInstigatorController(),
				Zelda::Channel::Damage
			);

			if (!bHit)
			{
				// 더 이상 닿아있지 않으면 제거
				EStat.SurfaceHitArray.RemoveAtSwap(i, 1, EAllowShrinking::No);
			}
		}
	}
}

void UZCStateComponent::CCElementFX(bool bEnabled)
{
	if (!VFXComponent) return;

	if (bEnabled)
	{
		const FZCLoadedElementAssetData* ElementAsset = WorldSubsystem->GetElementAssetMap()->Find(Stagger.ElementStagger.CurrentElementTag);

		// ElementAsset이 nullptr이거나 VFX가 로드되지 않았을 경우 체크
		if (!ElementAsset || !ElementAsset->CharacterVisuals.VFX.IsValid()) return;

		if (VFXComponent->GetAsset() != ElementAsset->CharacterVisuals.VFX.Get())
		{
			VFXComponent->SetAsset(ElementAsset->CharacterVisuals.VFX.Get());
			VFXComponent->Activate(true);
		}
	}
	else
	{
		VFXComponent->Deactivate();
		if (OwnerCharacter && OwnerCharacter->GetMesh())
		{
			OwnerCharacter->GetMesh()->SetOverlayMaterial(nullptr);
		}
	}
}
