// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Physics/ZCPhysActorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCPhysActorComponent)

// Sets default values for this component's properties
UZCPhysActorComponent::UZCPhysActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// 자동 활성화
	bAutoActivate = true;

	// 시작할때 자동으로 Owner의 RootComponent를 등록할지 여부, InitializeComponent에서 수행
	bAutoRegisterUpdatedComponent = true;

	//UpdatedComponent가 유효하면 자동으로 틱을 활성화할 지 여부, UpdateTickRegistration에서 수행
	bAutoUpdateTickRegistration = true;

}


// Called when the game starts
void UZCPhysActorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UZCPhysActorComponent::InitializeComponent()
{
	Super::InitializeComponent();


}

void UZCPhysActorComponent::Activate(bool bReset)
{
	SetActiveFlag(true);
}

void UZCPhysActorComponent::Deactivate()
{
	SetActiveFlag(false);
}

// Called every frame
void UZCPhysActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdatedComponent || !UpdatedComponent->IsSimulatingPhysics()) return;

	const FPhysicsSetting& CurrentSetting = IsActive() ? ActiveSetting : PassiveSetting;
	CustomPhysics(DeltaTime, CurrentSetting);
}

void UZCPhysActorComponent::CustomPhysics(float DeltaTime, const FPhysicsSetting& Setting)
{
	EZCPhysicsFlags Flags = static_cast<EZCPhysicsFlags>(Setting.PhysicsFlags);

	// 기본 시뮬레이션
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::Default))
	{
		UpdatedPrimitive->SetSimulatePhysics(true);
	}
	else
	{
		UpdatedPrimitive->SetSimulatePhysics(false);
	}

	// 중력
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::Gravity))
	{
		ApplyGravity(Setting);
	}

	// 가속
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::Acceleration))
	{
		ApplyAcceleration(DeltaTime, Setting);
	}

	// 균형잡기
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::SelfBalancing))
	{
		ApplySelfBalancing(DeltaTime, Setting);
	}

	// 회전
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::Rotate))
	{
		ApplyRotate(DeltaTime, Setting);
	}

	// 추적
	if (EnumHasAllFlags(Flags, EZCPhysicsFlags::Tracking))
	{
		ApplyTracking(DeltaTime, Setting);
	}
}

void UZCPhysActorComponent::ApplyGravity(const FPhysicsSetting& Setting)
{
	switch (Setting.Gravity)
	{
	case EZCGravityType::Default:
		UpdatedPrimitive->SetEnableGravity(true);
		break;

	case EZCGravityType::LowGravity:
	{
		UpdatedPrimitive->SetEnableGravity(false);
		const float Scale = 0.3f;
		const FVector LowGravity = FVector(0.0f, 0.0f, GetGravityZ() * Scale);

		UpdatedPrimitive->AddForce(LowGravity * UpdatedPrimitive->GetMass());
		break;
	}
		
	case EZCGravityType::NoGravity:
		UpdatedPrimitive->SetEnableGravity(false);
		break;

	}
}

void UZCPhysActorComponent::ApplyAcceleration(float DeltaTime, const FPhysicsSetting& Setting)
{
	const EZCAcceleration AccelFlags = static_cast<EZCAcceleration>(Setting.Acceleration);

	if (AccelFlags == EZCAcceleration::None) return;

	const AActor* Owner = GetOwner();

	FVector Dir = FVector::ZeroVector;

	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Right = Owner->GetActorRightVector();
	const FVector Up = Owner->GetActorUpVector();

	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Forward))		Dir += Forward;
	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Backward))		Dir -= Forward;
	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Rightward))	Dir += Right;
	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Leftward))		Dir -= Right;
	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Upward))		Dir += Up;
	if (EnumHasAnyFlags(AccelFlags, EZCAcceleration::Downward))		Dir -= Up;

	Dir.Normalize();

	const float AccelStrength = 200.0f;

	const FVector Force = Dir * AccelStrength * UpdatedPrimitive->GetMass();
	UpdatedPrimitive->AddForce(Force);
}

void UZCPhysActorComponent::ApplySelfBalancing(float DeltaTime, const FPhysicsSetting& Setting)
{
}

void UZCPhysActorComponent::ApplyRotate(float DeltaTime, const FPhysicsSetting& Setting)
{
}

void UZCPhysActorComponent::ApplyTracking(float DeltaTime, const FPhysicsSetting& Setting)
{
}

