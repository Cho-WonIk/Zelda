// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ZCDeferredForcesGear.h"

#include "Chaos/Particle/ParticleUtilities.h"
#include "Chaos/Utilities.h"
#include "Chaos/ParticleHandle.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "PhysicsProxy/ClusterUnionPhysicsProxy.h"
#include "Chaos/ClusterUnionManager.h"

#if CHAOS_DEBUG_DRAW
#include "Core/ZCGearUtility.h"
#endif

struct FZCGearCoreDebugParams
{
	bool ShowMass = false;
	bool ShowForces = false;
	bool ShowTorques = false;
};

FZCGearCoreDebugParams GCoreGearDebugParams;

#if CHAOS_DEBUG_DRAW
static FAutoConsoleVariable CVarShowGearForces(TEXT("zc.gear.forces"), false, TEXT("힘 시각화"), ECVF_Default);
static FAutoConsoleVariable CVarShowGearTorques(TEXT("zc.gear.torques"), false, TEXT("토크 시각화"), ECVF_Default);
#endif


FZCDeferredForcesGear::FZCDeferredForcesGear()
{
}

FZCDeferredForcesGear::~FZCDeferredForcesGear()
{
}

void FZCDeferredForcesGear::Apply()
{
	// 모든 지연된 힘 적용
	for (const auto& Data : ApplyForceDatas)			Internal_ApplyForce(Data);
	for (const auto& Data : ApplyForceAtCOMDatas)		Internal_ApplyForceAtPosition(Data, true);
	for (const auto& Data : ApplyForceAtPositionDatas)	Internal_ApplyForceAtPosition(Data, false);
	for (const auto& Data : ApplyTorqueDatas)			Internal_ApplyTorque(Data);
	for (const auto& Data : SetVelocityDatas)			Internal_ApplySetVelocity(Data);
	for (const auto& Data : SetAngularVelocityDatas)	Internal_ApplySetAngularVelocity(Data);
	for (const auto& Data : SetRotationDatas)			Internal_ApplySetRotation(Data);

	for (const auto& Data : SetTorqueDatas)				Internal_ApplySetTorque(Data);

	for (const auto& Data : ApplySpeedLimitedForceDatas)Internal_ApplySpeedLimitedForce(Data);

	ApplyForceDatas.Reset();
	ApplyForceAtCOMDatas.Reset();
	ApplyForceAtPositionDatas.Reset();
	ApplyTorqueDatas.Reset();

	SetVelocityDatas.Reset();
	SetAngularVelocityDatas.Reset();
	SetRotationDatas.Reset();

	SetTorqueDatas.Reset();

	ApplySpeedLimitedForceDatas.Reset();
}

Chaos::FPBDRigidParticleHandle* FZCDeferredForcesGear::GetParticleHandleFromProxy(IPhysicsProxyBase* Proxy) const
{
	if (!Proxy) return nullptr;

	switch (Proxy->GetType())
	{
	case EPhysicsProxyType::ClusterUnionProxy:
	{
		if (Chaos::FClusterUnionPhysicsProxy* CUProxy = static_cast<Chaos::FClusterUnionPhysicsProxy*>(Proxy))
		{
			// Evolution 객체, 전체 물리 시뮬레이션의 변화시키는 역할을 담당하는 객체
			Chaos::FPBDRigidsEvolutionGBF& Evolution = *static_cast<Chaos::FPBDRigidsSolver*>(CUProxy->GetSolver<Chaos::FPBDRigidsSolver>())->GetEvolution();
			// 클러스터 유니언 객체를 관리하는 매니저
			Chaos::FClusterUnionManager& ClusterUnionManager = Evolution.GetRigidClustering().GetClusterUnionManager();
			// 현재 프록시와 연결된 클러스터 유니언의 인덱스를 얻음
			const Chaos::FClusterUnionIndex& CUIndex = CUProxy->GetClusterUnionIndex();
			// 해당 인덱스를 통해 클러스터 유니언 객체를 찾음
			if (Chaos::FClusterUnion* ClusterUnion = ClusterUnionManager.FindClusterUnion(CUIndex))
			{
				// 클러스터 유니언에서 내부 클러스터 핸들을 얻음
				if (Chaos::FPBDRigidClusteredParticleHandle* ClusterHandle = ClusterUnion->InternalCluster)
				{
					return ClusterHandle;
				}
			}
		}
	}
	break;

	case EPhysicsProxyType::SingleParticleProxy:
	{
		if (Chaos::FSingleParticlePhysicsProxy* ParticleProxy = static_cast<Chaos::FSingleParticlePhysicsProxy*>(Proxy))
		{
			auto* HandleLowLevel = ParticleProxy->GetHandle_LowLevel();
			if (HandleLowLevel)
			{
				if (Chaos::FPBDRigidParticleHandle* RigidHandle = HandleLowLevel->CastToRigidParticle())
				{
					return RigidHandle;
				}
			}
		}
	}
	break;

	default:
		break;
	}

	return nullptr;
}

void FZCDeferredForcesGear::WakeUpParticle(Chaos::FPBDRigidParticleHandle* RP, IPhysicsProxyBase* Proxy)
{
	if (!RP || !Proxy) return;

	// 현재 잠자는 상태인지 확인
	if (RP->ObjectState() == Chaos::EObjectStateType::Sleeping)
	{
		// 1. 파티클 자체의 상태를 Dynamic으로 변경 (LowLevel)
		RP->SetObjectStateLowLevel(Chaos::EObjectStateType::Dynamic);

		// 2. Solver와 Evolution을 통해 파티클을 활성 리스트로 이동
		if (auto* Solver = static_cast<Chaos::FPBDRigidsSolver*>(Proxy->GetSolver<Chaos::FPBDRigidsSolver>()))
		{
			if (auto* Evolution = Solver->GetEvolution())
			{
				// Evolution 레벨에서 상태를 전이시켜야 시뮬레이션 루프에 포함됨
				Evolution->SetParticleObjectState(RP, Chaos::EObjectStateType::Dynamic);
			}
		}
	}
}

void FZCDeferredForcesGear::Internal_ApplyForce(const FZCApplyForceData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		const Chaos::FVec3 WorldCOM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RP);

		// 힘 벡터 계산
		Chaos::FVec3 ForceWorld = Data.Force;

		// 로컬 힘인 경우: Offset 회전 -> Actor 회전 적용
		if (EnumHasAllFlags(Data.Flags, EZCForceFlags::IsLocalForce))
		{
			ForceWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Force));
		}
		// 월드 힘인 경우: Data.Force 그대로 사용 (방향 고정)

		// 작용점 위치 계산 (위치는 항상 Actor 기준 로컬 오프셋을 월드로 변환)
		Chaos::FVec3 PositionWorld = WorldTM.TransformPosition(Data.OffsetTransform.GetLocation());

		// 회전 팔(Arm) 계산 및 적용
		Chaos::FVec3 Arm = PositionWorld - WorldCOM;
		RP->AddForce(ForceWorld);
		RP->AddTorque(Chaos::FVec3::CrossProduct(Arm, ForceWorld));

#if CHAOS_DEBUG_DRAW
		using namespace ZCGear;
		if (GCoreGearDebugParams.ShowForces)
		{
			// 힘의 작용점에 작은 구체 표시
			FZCDebugDraw::Sphere(PositionWorld, 5.0f, Data.DebugColor, 0.0f);
			// 힘의 방향과 크기를 화살표로 표시 (크기 조절을 위해 0.01f 곱함)
			FZCDebugDraw::Arrow(PositionWorld, PositionWorld + (ForceWorld * 0.01f), 10.0f, Data.DebugColor, 2.0f, 0.0f);
		}
#endif
	}
}

void FZCDeferredForcesGear::Internal_ApplyForceAtPosition(const FZCApplyForceAtPositionData& Data, bool bIsCOM)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		Chaos::FVec3 ForceWorld = Data.Force;

		// 좌표 변환 : Local -> World
		if (EnumHasAllFlags(Data.Flags, EZCForceFlags::IsLocalForce))
		{
			ForceWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Force));
		}
		Chaos::FVec3 WorldArm;

		if (bIsCOM)
		{
			WorldArm = WorldTM.TransformVector(Data.Position);
		}
		else
		{
			Chaos::FVec3 LocalPos = Data.OffsetTransform.TransformPosition(Data.Position);
			Chaos::FVec3 ArmLocal = LocalPos - RP->CenterOfMass();
			WorldArm = WorldTM.TransformVector(ArmLocal);
		}

		RP->AddForce(ForceWorld);
		RP->AddTorque(Chaos::FVec3::CrossProduct(WorldArm, ForceWorld));
	}
}

void FZCDeferredForcesGear::Internal_ApplyTorque(const FZCAddTorqueData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		// Torque 변환: Local -> Offset -> World
		Chaos::FVec3 TorqueWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Torque));
		RP->AddTorque(TorqueWorld);

#if CHAOS_DEBUG_DRAW
		using namespace ZCGear;
		if (GCoreGearDebugParams.ShowTorques)
		{
			// 토크는 회전이므로 CoM에서 화살표로 표시 (보라색 계열 추천)
			Chaos::FVec3 CoM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RP);
			FZCDebugDraw::Arrow(CoM, CoM + (TorqueWorld * 0.001f), 15.0f, Data.DebugColor, 3.0f, 0.0f);
		}
#endif
	}
}

void FZCDeferredForcesGear::Internal_ApplySetVelocity(const FZCSetVelocityData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		Chaos::FVec3 VelWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Velocity));

		if (Data.bAddToCurrent) RP->SetV(RP->GetV() + VelWorld);
		else RP->SetV(VelWorld);
	}
}

void FZCDeferredForcesGear::Internal_ApplySetAngularVelocity(const FZCSetAngularVelocityData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());

		FVector W1 = Data.OffsetTransform.TransformVector(Data.AngularVelocity);
		Chaos::FVec3 WorldAngularVelocity = WorldTM.TransformVector(W1);

		if (Data.bAddToCurrent)
		{
			RP->SetW(RP->GetW() + WorldAngularVelocity);
		}
		else
		{
			RP->SetW(WorldAngularVelocity);
		}

#if CHAOS_DEBUG_DRAW
		using namespace ZCGear;
		// 토크와 구분하기 위해 콘솔 변수를 공유하거나 별도의 파라미터를 사용할 수 있습니다.
		if (GCoreGearDebugParams.ShowTorques)
		{
			const Chaos::FVec3 WorldCOM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RP);
			const Chaos::FVec3 FinalW = RP->GetW();

			// 각속도 벡터의 크기가 매우 작을 수 있으므로 시각적 확인을 위해 적절한 배율(예: 10.0f)을 곱합니다.
			// 화살표는 질량 중심에서 회전축 방향으로 그려집니다.
			FZCDebugDraw::Arrow(WorldCOM, WorldCOM + (FinalW * 10.0f), 12.0f, Data.DebugColor, 2.5f, 0.0f);

			// 회전 중심에 작은 점 표시
			FZCDebugDraw::Sphere(WorldCOM, 4.0f, Data.DebugColor, 0.0f);
		}
#endif
	}
}

void FZCDeferredForcesGear::Internal_ApplySetRotation(const FZCSetRotationData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		if (Data.bAddToCurrent)
		{
			// 현재 회전에 추가 (쿼터니언 곱셈)
			const Chaos::FRotation3 CurrentRot = RP->GetR();
			RP->SetR(CurrentRot * Data.Rotation);
		}
		else
		{
			// 절대값으로 설정
			RP->SetR(Data.Rotation);
		}
	}
}

void FZCDeferredForcesGear::Internal_ApplySetTorque(const FZCSetTorqueData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		Chaos::FVec3 TorqueWorld = Data.Torque;

		// 로컬 좌표계인 경우 월드 좌표계로 변환
		if (Data.bIsLocal)
		{
			TorqueWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Torque));
		}

		// 기존 토크를 무시하고 새로운 토크로 설정
		// SetTorque를 직접 지원하지 않는 경우 각속도를 설정하여 토크 효과를 냄
		// Torque = I * AngularAcceleration이므로, AngularVelocity를 직접 설정하여 회전 제어
		// 여기서는 Torque 방향으로 각속도를 설정하여 강제 회전 효과 구현
		RP->SetTorque(TorqueWorld);

#if CHAOS_DEBUG_DRAW
		using namespace ZCGear;
		if (GCoreGearDebugParams.ShowTorques)
		{
			Chaos::FVec3 CoM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RP);
			FZCDebugDraw::Arrow(CoM, CoM + (TorqueWorld * 0.001f), 15.0f, Data.DebugColor, 3.0f, 0.0f);
		}
#endif
	}
}

void FZCDeferredForcesGear::Internal_ApplySpeedLimitedForce(const FZCApplySpeedLimitedForceData& Data)
{
	Chaos::FPBDRigidParticleHandle* Handle = GetParticleHandleFromProxy(Data.Proxy);
	if (!Handle) return;

	if (Chaos::FPBDRigidParticleHandle* RP = Handle->CastToRigidParticle())
	{
		WakeUpParticle(RP, Data.Proxy);

		const FTransform WorldTM(RP->GetR(), RP->GetX());
		const Chaos::FVec3 WorldCOM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RP);

		// 힘 벡터 계산
		Chaos::FVec3 ForceWorld = Data.Force;

		// 로컬 힘인 경우: Offset 회전 -> Actor 회전 적용
		if (EnumHasAllFlags(Data.Flags, EZCForceFlags::IsLocalForce))
		{
			ForceWorld = WorldTM.TransformVector(Data.OffsetTransform.TransformVector(Data.Force));
		}

		// 방향 추출
		Chaos::FVec3 WorldForceDir = ForceWorld;
		if (!WorldForceDir.Normalize()) return;

		const Chaos::FVec3 CurrentVelocity = RP->GetV();
		float CurrentSpeedInDir = Chaos::FVec3::DotProduct(CurrentVelocity, WorldForceDir);

		// 속도 제한 체크: 현재 속도가 MaxSpeed보다 작을 때만 힘 적용
		if (CurrentSpeedInDir < Data.MaxSpeed)
		{
			// 작용점 위치 계산 (Local -> Offset -> World)
			Chaos::FVec3 PositionWorld = WorldTM.TransformPosition(Data.OffsetTransform.GetLocation());

			// 회전 팔(Arm) 계산 (작용점 - 질량중심)
			Chaos::FVec3 Arm = PositionWorld - WorldCOM;

			// 물리 적용
			RP->AddForce(ForceWorld);
			RP->AddTorque(Chaos::FVec3::CrossProduct(Arm, ForceWorld));

#if CHAOS_DEBUG_DRAW
			using namespace ZCGear;
			if (GCoreGearDebugParams.ShowForces)
			{
				// 작용점에 구체 표시
				FZCDebugDraw::Sphere(PositionWorld, 5.0f, Data.DebugColor, 0.0f);
				// 힘의 방향 화살표
				FZCDebugDraw::Arrow(PositionWorld, PositionWorld + (ForceWorld * 0.01f), 10.0f, Data.DebugColor, 2.0f, 0.0f);
			}
#endif
		}
	}
}
