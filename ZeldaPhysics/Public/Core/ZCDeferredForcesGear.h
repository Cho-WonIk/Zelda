// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ParticleHandleFwd.h"
#include "GeometryCollection/ManagedArray.h"

// 힘 플레그
enum class EZCForceFlags : uint32
{
	// 없음
	None					= 0,
	AllowSubstepping		= 1 << 0,
	AccelChange				= 1 << 1,
	VelChange				= 1 << 2,
	IsLocalForce			= 1 << 3,
	LevelSlope				= 1 << 4
};
ENUM_CLASS_FLAGS(EZCForceFlags);

class IPhysicsProxyBase;

class ZELDAPHYSICS_API FZCDeferredForcesGear
{
public:
	// 기록 데이터 구조체
	struct FZCBasePhysicsData
	{
		FZCBasePhysicsData(IPhysicsProxyBase* InProxy, const FTransform& InOffset)
			: Proxy(InProxy)
			, OffsetTransform(InOffset)
		{}

		IPhysicsProxyBase* Proxy;
		FTransform OffsetTransform;
	};

	// 힘 적용 데이터
	struct FZCApplyForceData : public FZCBasePhysicsData
	{
		FZCApplyForceData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InForce, bool bSubstep, bool bAccel, bool bLocal, bool bSlope, const FColor& InColor = FColor::Red)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Force(InForce)
			, Flags(EZCForceFlags::None)
			, DebugColor(InColor)
		{
			if (bSubstep) Flags |= EZCForceFlags::AllowSubstepping;
			if (bAccel)   Flags |= EZCForceFlags::AccelChange;
			if (bSlope)   Flags |= EZCForceFlags::LevelSlope;
			if (bLocal)   Flags |= EZCForceFlags::IsLocalForce;
		}
		FVector Force;
		EZCForceFlags Flags;
		FColor DebugColor;
	};

	// 위치에서 힘 적용 데이터
	struct FZCApplyForceAtPositionData : public FZCBasePhysicsData
	{
		FZCApplyForceAtPositionData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InForce, const FVector& InPos, bool bSubstep, bool bLocal, bool bSlope, const FColor& InColor = FColor::Red)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Force(InForce)
			, Position(InPos)
			, Flags(EZCForceFlags::None)
			, DebugColor(InColor)
		{
			if (bSubstep) Flags |= EZCForceFlags::AllowSubstepping;
			if (bLocal)   Flags |= EZCForceFlags::IsLocalForce;
			if (bSlope)   Flags |= EZCForceFlags::LevelSlope;
		}
		FVector Force;
		FVector Position;
		EZCForceFlags Flags;
		FColor DebugColor;
	};

	// 토크 추가 데이터
	struct FZCAddTorqueData : public FZCBasePhysicsData
	{
		FZCAddTorqueData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InTorque, bool bSubstep, bool bAccel, const FColor& InColor = FColor::Red)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Torque(InTorque)
			, Flags(EZCForceFlags::None)
			, DebugColor(InColor)
		{
			if (bSubstep) Flags |= EZCForceFlags::AllowSubstepping;
			if (bAccel)   Flags |= EZCForceFlags::AccelChange;
		}
		FVector Torque;
		EZCForceFlags Flags;
		FColor DebugColor;
	};

	// 강제로 해당 속도로 설정 데이터
	struct FZCSetVelocityData : public FZCBasePhysicsData
	{
		FZCSetVelocityData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InVel, bool bAdd, const FColor& InColor = FColor::Red)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Velocity(InVel)
			, bAddToCurrent(bAdd)
			, DebugColor(InColor)
		{
		}
		FVector Velocity;
		bool bAddToCurrent;
		FColor DebugColor;
	};

	// 강제로 해당 각속도로 설정 데이터
	struct FZCSetAngularVelocityData : public FZCBasePhysicsData
	{
		FZCSetAngularVelocityData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InAngVel, bool bAdd, const FColor& InColor)
			: FZCBasePhysicsData(InProxy, InOffset)
			, AngularVelocity(InAngVel)
			, bAddToCurrent(bAdd)
			, DebugColor(InColor)
		{
		}
		FVector AngularVelocity;
		bool bAddToCurrent;
		FColor DebugColor;
	};

	// 강제로 해당 회전값으로 설정 데이터
	struct FZCSetRotationData : public FZCBasePhysicsData
	{
		FZCSetRotationData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FQuat& InRot, bool bAdd)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Rotation(InRot)
			, bAddToCurrent(bAdd)
		{
		}
		FQuat Rotation;
		bool bAddToCurrent;
	};

	// 강제로 토크를 설정하는 데이터 (기존 토크를 무시하고 새 토크로 대체)
	struct FZCSetTorqueData : public FZCBasePhysicsData
	{
		FZCSetTorqueData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InTorque, bool bIsLocalTorque, const FColor& InColor = FColor::Yellow)
			: FZCBasePhysicsData(InProxy, InOffset)
			, Torque(InTorque)
			, bIsLocal(bIsLocalTorque)
			, DebugColor(InColor)
		{
		}
		FVector Torque;
		bool bIsLocal;
		FColor DebugColor;
	};


	// 속도 제한이 걸린 힘 적용 데이터
	struct FZCApplySpeedLimitedForceData : public FZCApplyForceData
	{
		FZCApplySpeedLimitedForceData(IPhysicsProxyBase* InProxy, const FTransform& InOffset, const FVector& InForce, const float MaxSpeed, bool bSubstep, bool bAccel, bool bLocal, bool bSlope, const FColor& InColor = FColor::Red)
			: FZCApplyForceData(InProxy, InOffset, InForce, bSubstep, bAccel, bLocal, bSlope, InColor)
			, MaxSpeed(MaxSpeed)
		{}

		float MaxSpeed;
	};
	
	FZCDeferredForcesGear();
	~FZCDeferredForcesGear();

	// 기록 함수 (Add)
	void Add(const FZCApplyForceData& Data)					{ ApplyForceDatas.Add(Data); }
	void Add(const FZCApplyForceAtPositionData& Data)		{ ApplyForceAtPositionDatas.Add(Data); }
	void AddCOM(const FZCApplyForceAtPositionData& Data)	{ ApplyForceAtCOMDatas.Add(Data); }
	void Add(const FZCAddTorqueData& Data)					{ ApplyTorqueDatas.Add(Data); }

	void Add(const FZCSetVelocityData& Data)				{ SetVelocityDatas.Add(Data); }
	void Add(const FZCSetAngularVelocityData& Data)			{ SetAngularVelocityDatas.Add(Data); }
	void Add(const FZCSetRotationData& Data)				{ SetRotationDatas.Add(Data); }

	void Add(const FZCSetTorqueData& Data)					{ SetTorqueDatas.Add(Data); }
	void Add(const FZCApplySpeedLimitedForceData& Data)		{ ApplySpeedLimitedForceDatas.Add(Data); }
	// 실행 함수
	void Apply();

private:
	// Proxy -> Handle 변환 헬퍼 함수
	Chaos::FPBDRigidParticleHandle* GetParticleHandleFromProxy(IPhysicsProxyBase* Proxy) const;

	// Sleeping 해제 헬퍼 함수
	void WakeUpParticle(Chaos::FPBDRigidParticleHandle* RP, IPhysicsProxyBase* Proxy);

	void Internal_ApplyForce(const FZCApplyForceData& Data);
	void Internal_ApplyForceAtPosition(const FZCApplyForceAtPositionData& Data, bool bIsCOM);
	void Internal_ApplyTorque(const FZCAddTorqueData& Data);

	void Internal_ApplySetVelocity(const FZCSetVelocityData& Data);
	void Internal_ApplySetAngularVelocity(const FZCSetAngularVelocityData& Data);
	void Internal_ApplySetRotation(const FZCSetRotationData& Data);

	void Internal_ApplySetTorque(const FZCSetTorqueData& Data);

	void Internal_ApplySpeedLimitedForce(const FZCApplySpeedLimitedForceData& Data);

	TArray<FZCApplyForceData> ApplyForceDatas;
	TArray<FZCApplyForceAtPositionData> ApplyForceAtCOMDatas;
	TArray<FZCApplyForceAtPositionData> ApplyForceAtPositionDatas;
	TArray<FZCAddTorqueData> ApplyTorqueDatas;

	TArray<FZCSetVelocityData> SetVelocityDatas;
	TArray<FZCSetAngularVelocityData> SetAngularVelocityDatas;
	TArray<FZCSetRotationData> SetRotationDatas;

	TArray<FZCSetTorqueData> SetTorqueDatas;

	TArray<FZCApplySpeedLimitedForceData> ApplySpeedLimitedForceDatas;
};
