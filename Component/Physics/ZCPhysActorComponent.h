// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "Physics/ZCPhysics.h"
#include "ZCPhysActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZELDA_API UZCPhysActorComponent : public UMovementComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCPhysActorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:	
	// 비활성화시 Passive상태, 활성화시 Active상태
	virtual void Activate(bool bReset = false) override;;
	virtual void Deactivate() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	void CustomPhysics(float DeltaTime, const FPhysicsSetting& Setting);

private:
	void ApplyGravity(const FPhysicsSetting& Setting);
	void ApplyAcceleration(float DeltaTime, const FPhysicsSetting& Setting);
	void ApplySelfBalancing(float DeltaTime, const FPhysicsSetting& Setting);
	void ApplyRotate(float DeltaTime, const FPhysicsSetting& Setting);
	void ApplyTracking(float DeltaTime, const FPhysicsSetting& Setting);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Passive 상태"))
	FPhysicsSetting PassiveSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Active 상태"))
	FPhysicsSetting ActiveSetting;

};

/*
날개 : 저중력 + 관성
수레 : 관성 + 마찰력 감소
열기구 : 위로 올라감
로켓 : 저중력 + 앞으로 나아감
커다란 타이어 : 축 중심 회전
작은 타이어 : 앞으로 나아감
썰매 : 관성 + 마찰력 감소
오뚝이 : 자체 균형
부유석 : 무중력
말뚝 : 지면 고정
추적 수레 : 추적 기능

*/

/*
조나우 기어 목록

날개				: 저중력, 관성
수레				: 관성, 마찰력 줄어듬(한쪽 면만)
열기구			: 불이 있으면 위로 올라감
로켓				: 진행방향으로 나아감
커다란 타이어		: 회전(축을 중심으로)
작은 타이어		: 앞으로 나아감
썰매				: 관성, 마찰 줄어듬
오뚝이			: Z축을 유지할려고함
부유석			: 무중력, 비행 상태
말뚝				: 지면에 고정됨
추적 수레		: 몬스터를 일정 시간마다 가장 가까운 적을 찾아 쫒아감


피직스 액터와 상관없는 것들
	선풍기
	타이머 폭탄
	휴대 냄비
	화룡의 머리
	빙룡의 머리
	뇌룡의 머리
	광선머리
	소화전
	조종간
	스페어 배터리
	대용량 배터리
	스프링
	대포
	라이트
	거울
	골렘의 머리
*/
