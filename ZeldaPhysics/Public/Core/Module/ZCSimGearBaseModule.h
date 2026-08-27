// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Chaos/GeometryParticlesfwd.h"
#include "Chaos/ParticleHandleFwd.h"
#include "Chaos/PhysicsObject.h"
#include "Logging/LogMacros.h"

#include "Core/ZCSimGearEnum.h"
#include "Core/ZCSimGearInput.h"

#define ZC_API ZELDAPHYSICS_API

class IPhysicsProxyBase;

struct FZCCoreGearModuleDebugParams
{
	bool ShowMass = false;
	bool ShowForces = false;
	float DrawForceScaling = 0.0004f;
	float LevelSlopeThreshold = 0.86f;
	bool DisableForces = false;
};

class FZCSimGearSimulation;

namespace ZCGear
{
	// 선언부
	class IZCSimGearBaseModule;


	// Dynamic_Cast와 비슷한 기능 구현
#define DECLARE_GEAR_MODULE_TYPE(ClassName) \
	static FName StaticGearModuleTypeName() {return FName(#ClassName); }

	// 베이스 타입 등록
	template<typename GearModuleClass>
	class TZCGearModuleTypeBase
	{
	public:
		TZCGearModuleTypeBase()
		{
			static_cast<GearModuleClass*>(this)->AddType(StaticGearModuleTypeBase());
		}

		static FName StaticGearModuleTypeBase()
		{
			return GearModuleClass::StaticGearModuleTypeName();
		}
	};

	template<typename _DerivedClass, typename ...Rest>
	class TZCGearModuleTypeable;

	// 파생 타입 등록(단일 상속)
	template<typename DerivedClass>
	class TZCGearModuleTypeable<DerivedClass>
	{
		public:
		TZCGearModuleTypeable()
		{
			static_cast<DerivedClass*>(this)->AddType(StaticGearModuleType());
		}
		static FName StaticGearModuleType()
		{
			return DerivedClass::StaticGearModuleTypeName();
		}
	};

	// 파생 타입 등록(명시적 부모 지정)
	template<typename DerivedClass, typename BaseClass>
	class TZCGearModuleTypeable<DerivedClass, BaseClass>
	{
		public:
		TZCGearModuleTypeable()
		{
			static_cast<BaseClass*>(this)->AddType(StaticGearModuleType());
		}
		static FName StaticGearModuleType()
		{
			return DerivedClass::StaticGearModuleTypeName();
		}
		static FName RecurseGearModuleType()
		{
			return StaticGearModuleType();
		}
	};

	// 재귀적 타입 체크
	template<class T, class = void>
	struct TZCIsRecursiveGearModuleType : std::false_type 
	{};

	template<class T>
	struct TZCIsRecursiveGearModuleType<T, std::enable_if_t<std::is_invocable_r<FName, decltype(T::RecurseGearModuleType)>::value>> : std::integral_constant<bool, true>
	{};

	template<class T>
	constexpr bool TZCIsRecursiveGearModuleType_v = TZCIsRecursiveGearModuleType<T>::value;

	// 기어 모듈 타입 컨테이너
	class FZCGearModuleTypeContainer
	{
	public:
		TSet<FName> GearModuleTypes;
		FName MostRecentAdd = NAME_None;

		void AddType(FName InType)
		{
			GearModuleTypes.Emplace(InType);
			MostRecentAdd = InType;
		}

		bool IsA(FName InType) const { return MostRecentAdd == InType || GearModuleTypes.Contains(InType); }

		FName GetType() const { return MostRecentAdd; }

		template<typename U>
		static FName GetStaticGearType()
		{
			if constexpr (TZCIsRecursiveGearModuleType_v<U>)
			{
				return U::RecurseGearModuleType();
			}
			else
			{
				return TZCGearModuleTypeable<U>::StaticGearModuleType();
			}
		}

		template<typename U>
		bool IsA() const
		{
			if constexpr (TZCIsRecursiveGearModuleType_v<U>)
			{
				return IsA(U::RecurseGearModuleType());
			}
			else
			{
				return IsA(TZCGearModuleTypeable<U>::StaticGearModuleType());
			}
		}

		template<typename U>
		U* Cast()
		{
			if (IsA<std::remove_const_t<U>>())
			{
				return static_cast<U*>(this);
			}
			return nullptr;
		}

		template<typename U>
		const U* Cast() const 
		{
			if (IsA<std::remove_const_t<U>>())
			{
				return static_cast<const U*>(this);
			}
			return nullptr;
		}
	};

	// 여기까지 Dynamic_Cast 대체 기능 구현


	namespace EAnimationFlags
	{
		static uint16 AnimateNone = 0x00000000;
		static uint16 AnimatePosition = 0x00000001;
		static uint16 AnimateRotation = 0x00000002;
	}

	// 모듈 설정값 래퍼
	template<typename T>
	class TZCSimGearModuleSettings
	{
	public:
		explicit TZCSimGearModuleSettings(const T& InSetup) : SetupData(InSetup)
		{
			SetupData = InSetup;
		}

		FORCEINLINE T& AccessSetup()
		{
			return (T&)(SetupData);
		}

		FORCEINLINE const T& Setup() const
		{
			return (SetupData);
		}
	private:
		T SetupData;
	};

	//-------------------------------------------------------

	// 물리 스레드에서 모듈에 전달되는 입력 데이터 구조체
	// FZCSimGearModuleInputContainer참조(블랙보드 객체)
	struct FZCSimGearAllInputs
	{
		FTransform GearWorldTransform;

		// 물리 스레드 전역 입력 컨테이너 참조 (블랙보드)
		const FZCSimGearModuleInputContainer* ModuleInputContainer = nullptr;

		bool HasValidInputContainer() const { return ModuleInputContainer != nullptr && ModuleInputContainer->bHasInput; }
	};

	// 기어 모듈 기능 플래그
	enum EZCSimGearModuleTypeFlags
	{
		// 기능 없음
		NonFunctional = (1 << 0),
		// 레이캐스트로 계산
		Raycast = (1 << 1),
		// 토크로 동작
		TorqueBased = (1 << 2),
		// 속도로 동작
		Velocity = (1 << 3),
	};

	// 시뮬레이션 결과 출력 데이터 구조체
	struct FZCSimGearOutputData : public FZCGearModuleTypeContainer, public TZCGearModuleTypeBase<FZCSimGearOutputData>
	{
		DECLARE_GEAR_MODULE_TYPE(FZCSimGearOutputData);

		FZCSimGearOutputData() = default;
		virtual ~FZCSimGearOutputData() {}

		virtual bool IsEnabled() const { return bEnabled; }

		virtual FZCSimGearOutputData* MakeNewData() = 0;

		// 모듈의 현재 상태로 출력 데이터 채우기
		ZC_API virtual void FillOutputState(const IZCSimGearBaseModule* Module);
		// 비동기 멀티스레드 환경에서 보간 처리
		ZC_API virtual void Lerp(const FZCSimGearOutputData& InCurrent, const FZCSimGearOutputData& InNext, float Alpha);

		bool bEnabled = true;
	};

	enum class EZCSimGearAxis : uint8
	{
		X = 0,
		Y = 1,
		Z = 2,
	};

	class IZCSimGearBaseModule : public FZCGearModuleTypeContainer, public TZCGearModuleTypeBase<IZCSimGearBaseModule>
	{
	public:
		DECLARE_GEAR_MODULE_TYPE(IZCSimGearBaseModule);

		IZCSimGearBaseModule()
		{
		}

		virtual ~IZCSimGearBaseModule() {}

	public:
		// 사용자 활성화 상태 설정 및 조회
		virtual void SetActiveState(EZCGearState NewState);// { ActiveState = NewState; }
		EZCGearState GetActiveState() const { return ActiveState; }

		// 현재 물리 시뮬레이션 상태 조회
		EZCSimGearPhysiscType GetCurrentPhysicsType() const { return (ActiveState == EZCGearState::Enabled) ? EnabledPhysicsType : DisabledPhysicsType; }

		// 활성화시 물리 시뮬레이션 타입 설정 및 조회
		virtual void SetEnabledPhysicsType(EZCSimGearPhysiscType NewType) { EnabledPhysicsType = NewType; }
		EZCSimGearPhysiscType GetEnabledPhysicsType() const { return EnabledPhysicsType; }

		// 비활성화시 물리 시뮬레이션 타입 설정 및 조회
		virtual void SetDisabledPhysicsType(EZCSimGearPhysiscType NewType) { DisabledPhysicsType = NewType; }
		EZCSimGearPhysiscType GetDisabledPhysicsType() const { return DisabledPhysicsType; }

		// 현재 로컬 선속도 적용 및 조회(게임 스레드에서 받아옴)
		void SetLocalLinearVelocity(const FVector& VelocityIn) { LocalLinearVelocity = VelocityIn; }
		const FVector& GetLocalLinearVelocity() const { return LocalLinearVelocity; }

		// 현재 로컬 각속도 적용 및 조회(게임 스레드에서 받아옴)
		void SetLocalAngularVelocity(const FVector& VelocityIn) { LocalAngularVelocity = VelocityIn; }
		const FVector& GetLocalAngularVelocity() const { return LocalAngularVelocity; }

		// 소유 시뮬레이션 설정
		void SetOwnerSimulation(FZCSimGearSimulation* InSimulation) { OwnerGearSimulation = InSimulation; }
		FZCSimGearSimulation* GetOwnerSimulation() const { return OwnerGearSimulation; }

		void SetRelativeTransform(const FTransform& InTransform) { RelativeTransform = InTransform; }
		const FTransform& GetRelativeTransform() const { return RelativeTransform; }

		void SetPhysicsProxy(IPhysicsProxyBase* Proxy);

		// 모듈이 생성될 때 외부에서 초기화할 내용
		virtual void OnConstruction_External(const Chaos::FPhysicsObjectHandle& PhysicsObject) {}
		// 모듈이 소멸될 때 외부에서 정리할 내용
		virtual void OnTermination_External() {}

		// 커스텀 물리 피직스, 물리 스레드에서 실행(비동기 콜백 함수를 통해 호출)
		virtual void Simulate(float DeltaTime, const FZCSimGearAllInputs& Inputs) = 0;

	public:
		ZC_API Chaos::FPBDRigidParticleHandle* GetParticleHandle(IPhysicsProxyBase* Proxy) const;

		 /*
		 모듈의 기준점으로부터 Position만큼 떨어진 지점에 힘을 가함
		 @param Force				: 적용할 힘 벡터
		 @param Position			: 힘이 적용될 상대적 위치	벡터
		 @param bAllowSubstepping	: 물리 서브스테핑 허용 여부(더 정밀한 계산이 필요할 경우 true)
		 @param bIsLocalForce		: Force 벡터가 로컬 좌표계인지 여부
		 @param bLevelSlope			: 지형 경사면 보정 적용 여부
		 @param DebugColorIn		: 디버그 드로잉 색상
		 */
		ZC_API void AddLocalForceAtPosition(const FVector& Force, const FVector& Position, bool bAllowSubstepping = true, bool bIsLocalForce = false, bool bLevelSlope = false, const FColor& DebugColorIn = FColor::Blue);

		/*
		물체의 실제 물리	중심점(Center of Mass)으로부터 OffsetFromCOM만큼 떨어진 지점에 힘을 가함
		@param Force				: 적용할 힘 벡터
		@param OffsetFromCOM		: 물리 중심점으로부터의 오프셋 거리
		@param bAllowSubstepping	: 물리 서브스테핑 허용 여부(더 정밀한 계산이 필요할 경우 true)
		@param bLevelSlope			: 지형 경사면 보정 적용 여부
		@param DebugColorIn			: 디버그 드로잉 색상
		*/
		ZC_API void AddForceAtCOMPosition(const FVector& Force, const FVector& OffsetFromCOM = FVector::ZeroVector, bool bAllowSubstepping = true, bool bLevelSlope = false, const FColor& DebugColorIn = FColor::Blue);

		/*
		모듈의 현재 트랜스폼 위치에 힘을 가함
		@param Force				: 적용할 힘 벡터
		@param bAllowSubstepping	: 물리 서브스테핑 허용 여부(더 정밀한 계산이 필요할 경우 true)
		@param bIsLocalForce		: Force 벡터가 로컬 좌표계인지 여부
		@param bLevelSlope			: 지형 경사면 보정 적용 여부
		*/
		ZC_API void AddLocalForce(const FVector& Force, bool bAllowSubstepping = true, bool bIsLocalForce = false, bool bLevelSlope = false, const FColor& DebugColorIn = FColor::Blue);

		/*
		토크를 적용시키는 함수, 회전, 롤/피치 제어에 사용
		@param Torque				: 적용할 토크 벡터(회전축 방향과 크기)
		@param bAllowSubstepping	: 물리 서브스테핑 허용 여부(더 정밀한 계산이 필요할 경우 true)
		@param bAccelChangeIn		: 각가속도를 직접 변경할지 여부(기본값 true)
		@param DebugColorIn			: 디버그 드로잉 색상
		*/
		ZC_API void AddLocalTorque(const FVector& Torque, bool bAllowSubstepping = true, bool bAccelChangeIn = true, const FColor& DebugColorIn = FColor::Magenta);

		/*
		선속도를 설정하는 함수, 이동 제어에 사용
		@param Velocity				: 설정할 속도 벡터
		@param bAddToCurrent		: 현재 속도에 더할지 여부(기본값 false)
		@param DebugColorIn			: 디버그 드로잉 색상
		*/
		ZC_API void SetVelocity(const FVector& Velocity, bool bAddToCurrent = false, const FColor& DebugColorIn = FColor::Cyan);

		/*
		각속도를 설정하는 함수, 회전 제어에 사용
		@param AngularVelocity		: 설정할 각속도 벡터
		@param bAddToCurrent		: 현재 각속도에 더할지 여부(기본값 false)
		@param DebugColorIn			: 디버그 드로잉 색상
		*/
		ZC_API void SetAngularVelocity(const FVector& AngularVelocity, bool bAddToCurrent = false, const FColor& DebugColorIn = FColor::Cyan);

		/*
		회전값을 설정하는 함수, 회전 제어에 사용
		@param Rotation				: 설정할 회전 쿼터니언
		@param bAddToCurrent		: 현재 회전에 더할지 여부(기본값 false)
		*/
		ZC_API void SetRotation(const FQuat& Rotation, bool bAddToCurrent = false);

		/*
		토크를 강제로 설정하는 함수 (기존 토크를 무시하고 새로운 토크로 대체)
		@param Torque				: 설정할 토크 벡터(회전축 방향과 크기)
		@param bIsLocalTorque		: Torque 벡터가 로컬 좌표계인지 여부(기본값 true)
		@param DebugColorIn			: 디버그 드로잉 색상
		*/
		ZC_API void SetTorque(const FVector& Torque, bool bIsLocalTorque = true, const FColor& DebugColorIn = FColor::Yellow);

		/**
		 특정 로컬 방향으로 힘을 가하되, 해당 방향의 월드 속도가 MaxSpeed를 넘지 않도록 제어
		 @param Force				: 적용할 힘 벡터
		 @param MaxSpeed			: 제한할 최대 속도
		 @param bAllowSubstepping	: 물리 서브스테핑 허용 여부
		 @param bAccelChangeIn		: 가속도 변화량으로 적용할지 여부
		 @param bIsLocalForce		: Force 벡터가 로컬 좌표계인지 여부 (추가됨)
		 @param bLevelSlope			: 지형 경사면 보정 적용 여부
		 @param DebugColorIn		: 디버그 드로잉 색상
		 */
		ZC_API void AddSpeedLimitedForce(const FVector& Force, float MaxSpeed, bool bAllowSubstepping = true, bool bAccelChangeIn = false, bool bIsLocalForce = true, bool bLevelSlope = false, const FColor& DebugColorIn = FColor::Red);

	protected:
		// 사용자가 모듈을 활성화 했는지 여부
		EZCGearState ActiveState = EZCGearState::Enabled;

		// 활성화, 비활성화시 물리 시뮬레이션 타입
		EZCSimGearPhysiscType EnabledPhysicsType = EZCSimGearPhysiscType::CustomPhysics;
		EZCSimGearPhysiscType DisabledPhysicsType = EZCSimGearPhysiscType::EnginePhysics;

		// 현재 상태값(게임 스레드에서 받아옴)
		// 현재 선속도
		FVector LocalLinearVelocity;
		// 현재 각속도
		FVector LocalAngularVelocity;

		FZCSimGearSimulation* OwnerGearSimulation = nullptr;

		FTransform RelativeTransform = FTransform::Identity;

		IPhysicsProxyBase* CachedPhysicsProxy = nullptr;
	};
}

#undef ZC_API
