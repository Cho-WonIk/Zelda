// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZCPhysicsSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "젤다 - 물리 설정"))
class ZELDA_API UZCPhysicsSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// 연결된 물체 간 허용되는 회전 각도 (도 단위)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Limits")
	float MaxAngularLimit = 10.0f;

	/*===== 파괴 조건 =====*/

	// 선형(위치) 파괴 임계값 (이 힘을 넘으면 연결이 끊어짐)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Breaking")
	float LinearBreakThreshold = 5000.0f;

	// 회전(토크) 파괴 임계값 (이 힘을 넘으면 연결이 끊어짐)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Breaking")
	float AngularBreakThreshold = 5000.0f;

	/*===== 탄성 조건 =====*/

	// 선형(위치) 복원력 (값이 클수록 원래 위치로 강하게 돌아옴)
	UPROPERTY(EditAnywhere, Config, Category = "Constraint|Drive")
	float LinearPositionStrength = 1000.0f;

	// 선형(위치) 속도 저항 (값이 클수록 진동이 줄어듦)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Drive")
	float LinearVelocityStrength = 100.0f;

	// 회전(토크) 스프링 강도 (값이 클수록 원래 각도로 강하게 돌아옴)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Drive")
	float AngularPositionStrength = 1000.0f;

	// 회전(토크) 속도 저항 (값이 클수록 회전 진동이 빨리 멈춤)
	UPROPERTY(EditAnywhere, config, Category = "Constraint|Drive")
	float AngularVelocityStrength = 100.0f;
};
