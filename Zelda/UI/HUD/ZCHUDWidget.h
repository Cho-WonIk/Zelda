// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameData/Enum/ZCItemType.h"
#include "ZCHUDWidget.generated.h"

class AZCPlayerController;
class AZCPlayerState;
class UZCShortCutWidget;
class UZCFaceButtonWidget;
class UZCCrosshairWidget;
enum class EFaceButtonType : uint8;

namespace ZCHUD
{
	namespace ActionIndicator
	{
		enum Type
		{
			Climb		= 0,
			Glide		= 1,
			Falling		= 2,
			Ultrahand	= 3,
			Fuse		= 4,
			Ascend		= 5,
			Recall		= 6,
			Autobuild	= 7
		};

		namespace Skill
		{
			namespace Ultrahand
			{
				enum
				{
					Find				= 0,
					SelectedSingle		= 1,
					SelectedMulti		= 2,
					Manipulation		= 3
				};
			}
		}
	}

	namespace ShortCutType
	{
		enum Type
		{
			Normal = 0,
			Weapon = 1,
			Shield = 2
		};
	}

	namespace FaceButtonMode
	{
		enum Type
		{
			Normal			= 0,
			Ultrahand		= 1,
			Fuse			= 2,
			Ascend			= 3,
			Recall			= 4,
			Autobuild		= 5
		};
	}
}

UCLASS()
class ZELDA_API UZCHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

protected:
};

UCLASS()
class ZELDA_API UZCInGameHUDWidget : public UZCHUDWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void ShowActionIndicatorUI(int32 MainIdx, int32 SubIdx = INDEX_NONE);
	void HideActionIndicatorUI();

	// 페이스 버튼 UI
	void SetFaceButtonMode(int32 ModeIndex);
	void UpdateFaceButtonState(const EFaceButtonType CurrentButton, const FText& Text);
	void HideFaceButtonUI();

	// 숏컷 UI
	void ShowShortCutUI(int32 SwitcherIndex, const TArray<class UZCShortcutListObject*>& ItemList);
	void ScrollShortCutItemList(int32 Direction);
	void HideShortCutUI();

	int32 GetCurrentShortCutItemID() const;

	// 크로스 헤어 UI
	void ShowCrosshairUI();
	void HideCrosshairUI();
	void SetLockOnCrosshair(bool bIsLockOn);

protected:
	// 알림 위젯( 아이템 습득, 튜토리얼 설명 등등)

	// 미니맵 위젯

	// HP, MP, 스태미너 바 위젯

	// 숏컷 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> ShortCutSwitcher;

	TObjectPtr<UZCShortCutWidget> CurrentShortCutWidget;

	// 현재 상태에서 입력 이행 가능한 동작 UI 하단에 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> ActionIndicatorSwitcher;

	// 페이스 버튼 ABXY의 UI를 담는 스위처
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> FaceButtonSwitcher;

	// 현재 활성화된 페이스 버튼 위젯 캐싱
	TObjectPtr<UZCFaceButtonWidget> CurrentFaceButtonWidget;

	// Crosshair 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UZCCrosshairWidget> CrosshairWidget;
};
