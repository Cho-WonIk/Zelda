// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "ZCLockTargetComponent.generated.h"

/*
 * 락온 타겟 컴포넌트
 * - 락온 타겟을 설정하고, 락온 타겟을 바라보도록 회전하는 기능을 제공
 * - 락온 타겟을 전환하는 기능을 제공
 * - 락온 타겟의 위치에 락온 위젯을 표시하는 기능을 제공
 
 기본적으로 ZCLockSpringArmComponent에 지정된 위젯을 사용해 락온 상태를 시각적으로 표시하지만 내부적으로 선언된 위젯 컴포넌트로 표시 가능

 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLockTargetActivateEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLockTargetDeactivateEvent);

class UUserWidget;
class UWidgetComponent;
class UZCLockTargetComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZELDA_API UZCLockTargetComponent : public USphereComponent
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	UZCLockTargetComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TObjectPtr<class UWidgetComponent> LockIndicatorWidgetComponent;

public:
	// 락온 활성화 시 호출되는 델리게이트
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "LockTarget_Function")
	FLockTargetActivateEvent OnLockTargetActivate;

	// 락온 비활성화 시 호출되는 델리게이트
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "LockTarget_Function")
	FLockTargetDeactivateEvent OnLockTargetDeactivate;

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void Disable();

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "LockTarget_Function")
	void Hide();

public:
	bool IsDisabled() const { return bIsDisabled; }

protected:
	/*true일 경우 LockSpringArm컴포넌트의 설정을 사용하지 않고 DrawSize와 bDrawAtDesiredSize를 재정의*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (DisplayName = "설정 재정의"))
	bool bOverrideDefaultWidgetSetting = false;

	/* LockSpringArm컴포넌트의 설정을 사용하지 않을시 대체함*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (EditCondition = "bOverrideDefaultWidgetSetting", DisplayName = "사용할 위젯"))
	TSubclassOf<UUserWidget> LockIndicatorWidgetClass = nullptr;

	/* 자동 사이즈 조절*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (EditCondition = "bOverrideDefaultWidgetSetting", DisplayName = "자동 사이즈 조절 유무"))
	bool bDrawAtDesiredSize = false;

	/*위젯의 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (EditCondition = "bOverrideDefaultWidgetSetting", DisplayName = "위젯 크기"))
	FIntPoint DrawSize = FIntPoint(100, 100);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (DisplayName = "위젯의 스폰 위치"))
	float SpawnOffset = 50.0f;

	/*true인 경우 락온 불가, Enable과 Disable로 변경 가능*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool bIsDisabled = false;

};
