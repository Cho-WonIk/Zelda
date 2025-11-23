// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameData/Enum/ZCItemType.h"
#include "ZCHUDCommonWidget.generated.h"

class AZCPlayerController;
class AZCPlayerState;

class UZCShortCutCommonWidget;

enum EFaceButtonType : uint8;
struct FZCInventoryUIEntry;

// IDX는 블루프린트의 순서대로 이루어져있음
namespace Widget
{
	namespace ActionIndicatorIdx
	{
		constexpr int32 Climb = 0;
		constexpr int32 Glide = 1;
		constexpr int32 Falling = 2;

		//constexpr int32 Diving (다이빙)
		//constexpr int32 LeftHand (왼손 기능들)
		//constexpr int32 Throw (무기 던지기 키)
		//constexpr int32 Bow (활 기능들)
		//constexpr int32 Horse (말 탈때 기능, 처음 말 탔을때 길들이기, 말조작법)
		//constexpr int32 HoldItem(아이템을 손에 든 경우)
	}

	namespace ShortCutType
	{
		constexpr int32 Normal = 0;
		constexpr int32 Weapon = 1;
		constexpr int32 Shield = 2;
	}
}

UCLASS()
class ZELDA_API UZCHUDCommonWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
public:
	// 현재 상태에서 입력 이행 가능한 동작 UI
	void ShowActionIndicatorUI(int32 IndicatorIdx);
	void HideActionIndicatorUI();

	// 페이스 버튼 UI
	void ShowFaceButtonUI(const EFaceButtonType CurrentButton, const FText& Text);
	void HideFaceButtonUI();

	// 숏컷 UI
	void ShowShortCutUI(EItemType ItemType);
	void ScrollShortCutItemList(int32 Direction);
	void HideShortCutUI();
	
	int32 GetCurrentShortCutItemID() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = PlayerController)
	TObjectPtr<AZCPlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = PlayerState)
	TObjectPtr<AZCPlayerState> PlayerState; 

protected:
	// 알림 위젯( 아이템 습득, 튜토리얼 설명 등등)

	// 미니맵 위젯

	// HP, MP, 스태미너 바 위젯

	// 숏컷 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> ShortCutSwitcher;

	TObjectPtr<UZCShortCutCommonWidget> CurrentShortCutWidget;

	// 현재 상태에서 입력 이행 가능한 동작 UI 하단에 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> ActionIndicatorSwitcher;

	// 페이스 버튼 ABXY의 UI
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UZCFaceButtonWidget> FaceButtonWidget;

};
