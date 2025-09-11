// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/State/ZCStateComponent.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Development/ZCDebug.h"
#include "Component/Reaction/Struct/ZCReactionEnum.h"

#include "Development/ZCLogger.h"

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

	Health.Set(100.0f);
	Stagger.Set(50.0f, 0.0f);
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

	GetWorld()->GetTimerManager().ClearTimer(StaggerDecayTimer);

	bCanDecreaseStagger = true;

	Super::EndPlay(EndPlayReason);

}

void UZCStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//UpdateStagger(DeltaTime);

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

void UZCStateComponent::ApplyDamage(float DamageAmount, const FGameplayTag& ElementTag, const FGameplayTag& DamageTypeTag, bool IsCritialBone, bool UseHitCuaserDirection, const FHitResult& HitResult, const AActor* HitCauser)
{
	if (Health.IsZero()) return;

	EHitStrength HitStrength = EHitStrength::Light;

	//EElementRel ElementRel = UZCGameplayFunctionLibrary::ElementWeakness(ElementTag, WeakElementTag);

	//if (ElementRel == EElementRel::Weak) HitStrength = EHitStrength::Medium;
	if (IsCritialBone) HitStrength = EHitStrength::Heavy;

	float AmountDamage = DamageAmount;//UZCGameplayFunctionLibrary::CalculateDamage(DamageAmount, ElementRel, IsCritialBone, ArmorState);

	Health.Sub(AmountDamage);
	OnHealthChanged.Broadcast(Health);

	if (Health.IsZero()) { OnHealthZero.Broadcast(); }

	float StaggerAmount = AmountDamage * 0.1f;
	Stagger.Add(StaggerAmount);

	if (Stagger.IsFull())
	{
		HitStrength = EHitStrength::Explosion;
		OnStaggerFull.Broadcast();
	}

	UZCLogger::Warning(TEXT("AmountDamage : {0}, StaggerAmount : {1}"), AmountDamage, StaggerAmount);

	OnHit.Broadcast(HitCauser, HitResult, static_cast<uint8>(HitStrength), Health.IsZero(), UseHitCuaserDirection);

	// 데미지 전달(스탯컴포넌트에, 전달 값 : 데미지, 크리티컬 여부)
	// 캐릭터가 죽었는지 확인
	// 크리티컬 공격인지 확인(넉백 이루어짐)
	// 그로기 수치에 도달했는지 확인(넉백 이루어짐)
	// 히트 리액션 컴포넌트 전달(FHitResult, 공격 강도, 죽음 여부)


	// 결과를 통해 일반 공격인 경우 리액션, (죽음, 크리티걸 공격 혹은 그로기 수치 도달에 의한 리액션은 델리게이트에 바인딩)

	// 라이트		: 일반 공격
	// 미디엄		: 약점 속성
	// 헤비			: 크리티컬 공격
	// 폭발			: 폭발 + 그로기 수치 도달
}

//void UZCStateComponent::ApplyDamage(float DamageAmount, const FGameplayTag& ElementTag, const FGameplayTag& DamageTypeTag, bool IsCriticalBone)
//{
//	float FinalDamage = CalculateDamage(DamageAmount, ElementTag, DamageTypeTag, IsCriticalBone);
//	ApplyDamage(FinalDamage);
//
//	float FinalStagger = CalculateStagger(FinalDamage, ElementTag);
//	AddStaggerGauge(FinalStagger);
//}

//void UZCStateComponent::ApplyHeal(float HealAmount)
//{
//	Health.Add(HealAmount);
//
//	OnHealthChanged.Broadcast(Health);
//}
//
//void UZCStateComponent::AddStaggerGauge(float Value)
//{
//	Stagger.Add(Value);
//
//	// 그로기 게이지 감소 방지
//	bCanDecreaseStagger = false;
//
//	if (Stagger.IsFull()) OnStaggerFull.ExecuteIfBound();
//
//	GetWorld()->GetTimerManager().SetTimer(StaggerDecayTimer, FTimerDelegate::CreateLambda([this]() { bCanDecreaseStagger = true; }), StaggerDecayRate, false);
//}
//
//void UZCStateComponent::UpdateStagger(float DeltaTime)
//{
//	if (!bCanDecreaseStagger || Stagger.IsZero()) return;
//
//	Stagger.Sub(StaggerDecayAmount * DeltaTime);
//}
