// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Chemistry/ZCMaterialStateComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Gameplay/Damage/ZCDamage.h"
#include "Engine/OverlapResult.h"
#include "Physics/ZCCollision.h"
#include "Actor/ZCActor.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"


const FElementState FElementState::Empty = FElementState(FGameplayTag::EmptyTag, -1.0f, -1);

// Sets default values for this component's properties
UZCMaterialStateComponent::UZCMaterialStateComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.125f;

	bWantsInitializeComponent = true;
	// ...
}

void UZCMaterialStateComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//OwnerCasted = Cast<AZCActor>(GetOwner());
}

void UZCMaterialStateComponent::BeginPlay()
{
	Super::BeginPlay();

	//VFXComponentCached = OwnerCasted->GetZCNiagaraComponent();

	WorldSubsystem = GetWorld()->GetSubsystem<UZCWorldSubsystem>();

	WordSubsystemHandle = WorldSubsystem->OnAssetLoadFinished.AddLambda([&]() {
		MaterialData = WorldSubsystem->GetObjectMaterialInstanceData(Material);
		CurrentElementState = FElementState::Empty;
	});
}

void UZCMaterialStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WorldSubsystem->OnAssetLoadFinished.Remove(WordSubsystemHandle);
	WorldSubsystem = nullptr;
	ElementData = nullptr;
	MaterialData = nullptr;
	VFXComponentCached = nullptr;
	OwnerCasted = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UZCMaterialStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentElementState.IsEmpty())
	{
		if (CurrentElementState.Duration > 0.0f)
		{
			CurrentElementState.Duration = FMath::Max(0.0f, CurrentElementState.Duration - DeltaTime);



			if (CurrentElementState.Duration == 0.0f)
			{
				CurrentElementState.Reset();
				CurrentThresholdValueMap.Reset();
				ElementData = nullptr;

				LoopFX(false);
				EndFX(true);
				SetComponentTickEnabled(false);
			}
		}
		// CurrentEelementStat의 duration이 -1인 경우 무한이 지속

		// TODO : 액터 본인에게 데미지 주입 로직 작성(bIsDamageOnce가 false인 경우)

		// 주변에 전파
		ProcessElementSpreading();
	}
}

void UZCMaterialStateComponent::ApplyElementExposure(const FElementInfo& NewElementInfo)
{
	// 규칙
	// 1. 모든 전파는 Source에서 Target으로, 양방향 혹은 역전파는 일어나지 않는다.
	// 2. 우선순위 : 1순위(원소->원소) 2순위(원소->물질)
	// 3. 원소->물질의 경우 FMaterialCDO에 정의된 각 원소별 임계치를 초과해야 발생하게된다. 그전까지는 지속적으로 값을 주입
	// 4. 원소->원소의 경우 소스의 원소가 타겟의 원소에 영향을 준다.
	// 5. 확산 될때 SpreadingCount를 감소시키며 0에 도달하면 전파될 수 없다.(FElementCDO에 정의된 값을 넘어서 전파될 수 없다)(확산 중단)
	// 6. 확산 방식은 UZCMaterialStateComponent에서 지속적으로 Sphere Trace를 통해 전파
	// 7. 만약 규칙이 정해지지 않은 반응이면 아무런 효과도 없다.
	// 8. 확산 카운트와 경과시간을 통해 의도치 않은 역전파를 막는 로직을 추가
	// 9. bIsDamageOnce가 true여도 만약 원소의 지속성이 있으면 Tick을 통해 원소 전파가 가능함, 데미지와 함께 전달되지 않음

	if (!WorldSubsystem || !MaterialData) return;
	if (NewElementInfo.SpreadCount < 0) return;

	FReactionOut Out;

	// --------------------------
	// 1) 기존 원소가 있는 경우: 원소↔원소 반응 우선
	// --------------------------
	if (!CurrentElementState.IsEmpty())
	{
		// 이미 적용된 원소와 새로운 원소가 동일한 원소이고
		// 현재 적용된 원소값이 먼저(먼저 전파된 경우)
		// 역전파를 막음
		if (CurrentElementState.ElementTag == NewElementInfo.ElementTag && CurrentElementState.SpreadingCount > NewElementInfo.SpreadCount) return;

		// 추가적으로 불 -> 불 과 같은 로직을 안만들면 역전파가 안일어남 + 재 갱신 안일어남
		// 반응 미정의면 기존 유지
		if (!WorldSubsystem->TryGetObjectOutCome(NewElementInfo.ElementTag, CurrentElementState.ElementTag, Out)) return;

		// 상쇄(EmptyTag), 적용된 원소 제거
		if (Out.NewElementTag == FGameplayTag::EmptyTag)
		{
			CurrentElementState.Reset();
			CurrentThresholdValueMap.Reset();
			ElementData = nullptr;

			LoopFX(false);
			EndFX(true);

			SetComponentTickEnabled(false);
			return;
		}
	}
	// --------------------------
	// 2) 기존 원소가 없는 경우: 원소↔물질 반응 + 임계치 누적
	// --------------------------
	else
	{
		if (!WorldSubsystem->TryGetObjectOutCome(NewElementInfo.ElementTag, Material, Out)) return;
		// 상쇄(EmptyTag)면 적용 없음
		if (Out.NewElementTag == FGameplayTag::EmptyTag) return;

		// 결과 태그 기준으로 누적/판정

		// 만약 임계치가 없으면 미 반응이므로 무시
		const float Threshold = MaterialData->Threshold.FindRef(Out.NewElementTag);
		if (Threshold <= 0.0f) return;

		float& CurrentValue = CurrentThresholdValueMap.FindOrAdd(Out.NewElementTag);
		CurrentValue += MaterialData->ThresholdDelta.FindRef(Out.NewElementTag);

		//UZCLogger::Warning(TEXT("{0}에 원소 효과 누적되는 중 {1} / {2}"), *OwnerCasted->GetName(), CurrentValue, Threshold);

		// 임계치 미도달
		if (CurrentValue < Threshold) return;
	}

	// 임계 통과 → 적용
	ApplyElement(Out, NewElementInfo.SpreadCount - 1);
	// 원소가 적용되면 임계치 리셋
	CurrentThresholdValueMap.Reset();
}

#if WITH_EDITOR
void UZCMaterialStateComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (OwnerCasted)
	{
		if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UZCMaterialStateComponent, CurrentElementState))
		{
			FReactionOut TestOut;
			TestOut.NewElementTag = CurrentElementState.ElementTag;
			TestOut.Duration = -1.0f;
			TestOut.bIsDamageOnce = false;
			TestOut.ElementFirstDamage = 0.0f;
			TestOut.ElementTickDamage = 0.0f;

			ApplyElement(TestOut, 3);
		}
	}
}
#endif // WITH_EDITOR

void UZCMaterialStateComponent::ApplyElement(const FReactionOut& ReactionResult, const int32& SpreadingCount)
{
	ElementData = WorldSubsystem->GetObjectElementInstanceData(ReactionResult.NewElementTag);
	if (!ElementData) return;

	//UZCLogger::Warning(TEXT("{0}에 원소 반응이 트리거되어 원소가 적용됨, 적용된 원소 : {1}, 확산 카운트 : {2}"), *OwnerCasted->GetName(), ReactionResult.NewElementTag.GetTagName(), SpreadingCount);

	CurrentElementState.Init(ReactionResult, SpreadingCount, ElementData->Damage);

	if (CurrentElementState.bIsDamageOnce)
	{
		// TODO: 초기 데미지가 있는 경우 Owner 액터에 데미지 주입
		ReactionResult.ElementFirstDamage;
	}

	LoopFX(false);

	StartFX(true);

	if (CurrentElementState.Duration > 0.0f || CurrentElementState.Duration == -1.0f)
	{
		LoopFX(true);
		SetComponentTickEnabled(true);
	}
}

void UZCMaterialStateComponent::ProcessElementSpreading()
{
	if (!WorldSubsystem || CurrentElementState.IsEmpty()) return;
	if (!ElementData) return;

	AActor* Owner = GetOwner();

	if (!Owner) return;

	// 범위 내 액터들 엘리먼트 임계치 증가 시킴, 데미지가 0.0f이어도 확산이 일어나야 함.
	FElementInfo NewInfo(CurrentElementState.ElementTag, CurrentElementState.Duration, CurrentElementState.SpreadingCount);
	float ApplyDamage = CurrentElementState.ElementSpreadDamage;
	UZCGameplayFunctionLibrary::ApplyRadialDamage(NewInfo, this, ApplyDamage, Owner->GetActorLocation(), ElementData->SpreadingRange, UZCDamageType::StaticClass(), { Owner }, Owner, Owner->GetInstigatorController(), false, Zelda::Channel::Damage);

	//DrawDebugSphere(GetWorld(), OwnerCasted->GetActorLocation(), ElementData->SpreadingRange, 16, FColor::Green, false, 0.125f, 0, 0.5f);
}

void UZCMaterialStateComponent::StartFX(bool bEnabled)
{
	if (!VFXComponentCached || !ElementData || !ElementData->StartVFX) return;

	if (bEnabled)
	{
		if (ElementData->StartVFX)
		{
			VFXComponentCached->SetAsset(ElementData->StartVFX);
			VFXComponentCached->Activate(true);
		}
	}
	else
	{
		VFXComponentCached->Deactivate();
	}
}

void UZCMaterialStateComponent::LoopFX(bool bEnabled)
{
	if (!VFXComponentCached) return;

	if (bEnabled)
	{
		if (ElementData && ElementData->LoopVFX)
		{
			VFXComponentCached->SetAsset(ElementData->LoopVFX);
			VFXComponentCached->Activate(true);
		}
	}
	else
	{
		VFXComponentCached->Deactivate();
	}
}

void UZCMaterialStateComponent::EndFX(bool bEnabled)
{
	if (!VFXComponentCached || !ElementData) return;

	if (bEnabled)
	{
		if (ElementData->EndVFX)
		{
			VFXComponentCached->SetAsset(ElementData->EndVFX);
			VFXComponentCached->Activate(true);
		}
	}
	else
	{
		VFXComponentCached->Deactivate();
	}
}
