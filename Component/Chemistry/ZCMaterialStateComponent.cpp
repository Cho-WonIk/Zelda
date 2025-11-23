// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Chemistry/ZCMaterialStateComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameData/Struct/ZCChemistryStruct.h"
#include "World/Subsystem/ZCWorldSubsystem.h"
#include "Gameplay/ZCGameplayFunctionLibrary.h"
#include "Actor/ZCActor.h"
#include "Development/ZCLogger.h"
#include "Component/VFX/ZCNiagaraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCMaterialStateComponent)

UZCMaterialStateComponent::UZCMaterialStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UZCMaterialStateComponent::BeginPlay()
{
	Super::BeginPlay();

	WorldSubsystem = GetWorld()->GetSubsystem<UZCWorldSubsystem>();

	if (WorldSubsystem && OwnerCasted && VFXComponentCached)
	{
		// Subsystem에 등록하고 핸들 획득
		SystemHandle = WorldSubsystem->RegisterMaterial(OwnerCasted, VFXComponentCached, Material);
	}
}

void UZCMaterialStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Subsystem에서 등록 해제
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		WorldSubsystem->UnregisterMaterial(SystemHandle);
		SystemHandle = FMaterialHandle();
	}

	WorldSubsystem = nullptr;
	VFXComponentCached = nullptr;
	OwnerCasted = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UZCMaterialStateComponent::ApplyElementExposure(const FElementInfo& NewElementInfo)
{
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		WorldSubsystem->ApplyElementExposure(SystemHandle, NewElementInfo);
	}
}

void UZCMaterialStateComponent::NotifyHit(const FVector& Location, const FVector& Direction)
{
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		WorldSubsystem->NotifyHit(SystemHandle, Location, Direction);
	}
}

bool UZCMaterialStateComponent::HasActiveElement() const
{
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		const FMaterialRuntimeData* Data = WorldSubsystem->GetData(SystemHandle);
		return Data && !Data->IsEmpty();
	}
	return false;
}

FGameplayTag UZCMaterialStateComponent::GetCurrentElementTag() const
{
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		const FMaterialRuntimeData* Data = WorldSubsystem->GetData(SystemHandle);
		return Data ? Data->CurrentElementTag : FGameplayTag::EmptyTag;
	}
	return FGameplayTag::EmptyTag;
}

bool UZCMaterialStateComponent::IsElementActive() const
{
	if (WorldSubsystem && SystemHandle.IsValid())
	{
		const FMaterialRuntimeData* Data = WorldSubsystem->GetData(SystemHandle);
		return Data && Data->IsActive();
	}
	return false;
}

#if WITH_EDITOR
void UZCMaterialStateComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (OwnerCasted && WorldSubsystem && SystemHandle.IsValid())
	{
		if (PropertyChangedEvent.MemberProperty &&
			PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UZCMaterialStateComponent, DebugElementTag))
		{
			if (DebugElementTag.IsValid())
			{
				// 강제 적용할 반응 결과 생성
				FReactionOut TestOut;
				TestOut.NewElementTag = DebugElementTag;
				TestOut.Duration = -1.0f;           // 무한 지속 의도
				TestOut.bIsDamageOnce = false;
				TestOut.ElementFirstDamage = 10.0f; // 테스트용 데미지
				TestOut.ElementTickDamage = 5.0f;   // 테스트용 틱뎀

				// 기존: ApplyElementExposure(TestInfo); -> 내부 로직에 의해 Duration이나 적용 여부가 GIS 데이터에 종속됨

				// 변경: Subsystem에 새로 만든 강제 적용 함수 호출
				WorldSubsystem->DebugForceSetElement(SystemHandle, TestOut);
			}
		}
	}
}
#endif // WITH_EDITOR
