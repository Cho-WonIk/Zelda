// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Gear/ZCInputGearActor.h"
#include "Components/ArrowComponent.h"

#include "Character/Player/ZCPlayerCharacter.h"
#include "Player/ZCHUDLocalPlayerSubsystem.h"
#include "World/Subsystem/ZCUltrahandWorldSubsystem.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearMovementComponent.h"
#include "ZeldaPhysics/Public/Runtime/Gear/ZCGearInputComponent.h"
#include "ZeldaPhysics/Public/ZCSimGearManager.h"

AZCInputGearActor::AZCInputGearActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UZCGearInputComponent>(AZCActor::MeshComponentName))
{
	GearInputComponent = Cast<UZCGearInputComponent>(Mesh);

    // 화살표 컴포넌트 생성 및 설정
    RideDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RideDirectionArrow"));
    RideDirectionArrow->SetupAttachment(GetMesh());
    RideDirectionArrow->ArrowSize = 1.0f;
    RideDirectionArrow->ArrowColor = FColor::Yellow;
    RideDirectionArrow->SetRelativeLocation(FVector::ZeroVector);
    RideDirectionArrow->SetUsingAbsoluteScale(true);

    // 기본적으로 에디터와 게임 내에서 모두 보이도록 설정 (HiddenInGame = false가 기본)
    RideDirectionArrow->SetHiddenInGame(false);
}

void AZCInputGearActor::RideStart()
{
	TSet<AZCGearActor*> ConnectedGears = GetWorld()->GetSubsystem<UZCUltrahandWorldSubsystem>()->ActivateConnectedGears(this);

	TArray<UZCGearMovementComponent*> ConnectedMovements;
	ConnectedMovements.Reserve(ConnectedGears.Num());
	for (auto& Gear : ConnectedGears)
	{
		ConnectedMovements.Add(Gear->GetGearMovementComponent());
	}

	float TotalMass = 0.0f;
	FVector CenterOfMassOffset = CalculateCenterOfMassOffset(TotalMass);
	GearInputComponent->SetCenterOfMassOffset(CenterOfMassOffset, TotalMass);

    // 탑승 시작 시 화살표 숨김
    RideDirectionArrow->SetHiddenInGame(true);

	GearInputComponent->StartRide(ConnectedMovements);
}

void AZCInputGearActor::RideEnd()
{
	TSet<AZCGearActor*> DisconnectedGears = GetWorld()->GetSubsystem<UZCUltrahandWorldSubsystem>()->DeactivateConnectedGears(this);

	TArray<UZCGearMovementComponent*> DisconnectedMovements;
	DisconnectedMovements.Reserve(DisconnectedGears.Num());
	for (auto& Gear : DisconnectedGears)
	{
		DisconnectedMovements.Add(Gear->GetGearMovementComponent());
	}

    // 탑승 종료 시 화살표 다시 표시
    RideDirectionArrow->SetHiddenInGame(false);

	GearInputComponent->EndRide(DisconnectedMovements);
}

void AZCInputGearActor::GetPlayerInput(const FVector2D& RawInput)
{
	GearInputComponent->ApplyPlayerInput(RawInput);
}

void AZCInputGearActor::OnEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnEnterRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (AZCPlayerCharacter* PlayerCharacter = Cast<AZCPlayerCharacter>(OtherActor))
	{
		PlayerCharacter->SetRideGear(this);
		UZCHUDLocalPlayerSubsystem::Get(this).ShowFaceButtonEvent(EZCFaceButtonEvent::RideGear);
	}
}

void AZCInputGearActor::OnExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnExitRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (AZCPlayerCharacter* PlayerCharacter = Cast<AZCPlayerCharacter>(OtherActor))
	{
		PlayerCharacter->SetRideGear(nullptr);
		UZCHUDLocalPlayerSubsystem::Get(this).HideFaceButtonEvent(EZCFaceButtonEvent::RideGear);
	}

}

FVector AZCInputGearActor::CalculateCenterOfMassOffset(float& InMass)
{
    TSet<AZCActor*> ConnectedGroup = GetWorld()->GetSubsystem<UZCUltrahandWorldSubsystem>()->GetConnectedGroupSet(this);

    FVector WeightedTotalCOM = FVector::ZeroVector;
    float TotalMass = 0.0f;

    for (AZCActor* Actor : ConnectedGroup)
    {
        if (!Actor) continue;

        TArray<UMeshComponent*> MeshComponents;
        Actor->GetComponents<UMeshComponent>(MeshComponents);

        for (UMeshComponent* MeshComp : MeshComponents)
        {
            // 물리 상태가 생성되어 있고, 질량이 있는 경우만 계산
            if (MeshComp && MeshComp->IsPhysicsStateCreated())
            {
                float Mass = MeshComp->GetMass();
                if (Mass > 0.0f)
                {
                    // 월드 공간의 무게중심
                    FVector COM = MeshComp->GetCenterOfMass();
                    WeightedTotalCOM += (COM * Mass);
                    TotalMass += Mass;
                }
            }
        }
    }

    // 기본값은 제로 벡터
    FVector LocalOffset = FVector::ZeroVector;
    InMass = TotalMass;

    if (TotalMass > 0.0f)
    {
        // 1. 월드 기준 전체 무게중심
        FVector GroupWorldCOM = WeightedTotalCOM / TotalMass;

        // 2. 월드 좌표를 이 액터 기준의 로컬 좌표(오프셋)로 변환
        // 결과값인 LocalOffset은 이 액터의 위치에서 COM까지의 상대적 거리/방향입니다.
        LocalOffset = GetActorTransform().InverseTransformPosition(GroupWorldCOM);

        // 3. 시각화 (디버깅용)
#if !UE_BUILD_SHIPPING
// 무게 중심점 (빨간색 구체)
        //DrawDebugSphere(GetWorld(), GroupWorldCOM, 15.0f, 12, FColor::Red, false, 3.0f);
        // 기어에서 무게 중심까지의 선 (초록색)
        //DrawDebugLine(GetWorld(), GetActorLocation(), GroupWorldCOM, FColor::Green, false, 3.0f, 0, 1.5f);
#endif
    }

    return LocalOffset;
}

#if WITH_EDITOR
TArray<FString> AZCInputGearActor::GetSocketNames() const
{
    TArray<FString> SocketNames;

    SocketNames.Add(FName(NAME_None).ToString());

    TArray<FComponentSocketDescription> OutSockets;
	SocketNames.Reserve(OutSockets.Num());
	GetMesh()->QuerySupportedSockets(OutSockets);

    for (const FComponentSocketDescription& SocketDesc : OutSockets)
    {
        SocketNames.Add(SocketDesc.Name.ToString());
    }
    return SocketNames;
}
#endif // WITH_EDITOR