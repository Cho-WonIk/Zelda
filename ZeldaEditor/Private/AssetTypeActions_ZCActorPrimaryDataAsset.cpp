
#include "AssetTypeActions_ZCActorPrimaryDataAsset.h"
#include "Zelda/GameData/PrimaryData/ZCActorPrimaryDataAsset.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SWindow.h" // 커스텀 창을 위한 예시
#include "Widgets/Text/STextBlock.h"

/*================================지금은 사용 안함=========================================================================*/

UClass* FZCAssetTypeActions_ZCActorPrimaryDataAsset::GetSupportedClass() const
{
	return UZCActorPrimaryDataAsset::StaticClass();
}

void FZCAssetTypeActions_ZCActorPrimaryDataAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	TArray<TWeakObjectPtr<UObject>> WeakObjects;
	for (auto & Obj : InObjects)
	{
		WeakObjects.Add(Obj);
	}

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("매쉬 Shape툴 열기")), // 메뉴에 표시될 텍스트
		FText::FromString(TEXT("매쉬의 단순 콜리전 세팅 툴을 엽니다.")), // 툴팁
		FSlateIcon(), // 아이콘 (비워둬도 됨)
		FUIAction(FExecuteAction::CreateSP(this, &FZCAssetTypeActions_ZCActorPrimaryDataAsset::ExecuteAction, WeakObjects))
	);
}

uint32 FZCAssetTypeActions_ZCActorPrimaryDataAsset::GetCategories()
{
	return EAssetTypeCategories::None;
}

void FZCAssetTypeActions_ZCActorPrimaryDataAsset::ExecuteAction(TArray<TWeakObjectPtr<UObject>> InObjects)
{
	// TWeakObjectPtr을 UObject*로 변환 (안전하게)
	TArray<UZCActorPrimaryDataAsset*> SelectedAssets;
	for (const TWeakObjectPtr<UObject>& WeakObj : InObjects)
	{
		if (WeakObj.IsValid())
		{
			// GetSupportedClass() 덕분에 이 캐스트는 항상 성공합니다.
			if (UZCActorPrimaryDataAsset* Asset = Cast<UZCActorPrimaryDataAsset>(WeakObj.Get()))
			{
				SelectedAssets.Add(Asset);
			}
		}
	}

	if (SelectedAssets.Num() == 0)
	{
		return; // 선택된 애셋이 없음
	}

	// --- 여기에 "커스텀 창"을 띄우는 코드를 작성합니다. ---

	// SWindow 생성
	TSharedRef<SWindow> CustomWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("커스텀 에디터 창")))
		.ClientSize(FVector2D(400, 200))
		[
			// 1. SBorder를 추가하여 정렬 및 패딩을 제어합니다.
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("NoBorder")) // 테두리 없애기 (선택 사항)
				.HAlign(HAlign_Center) // <--- 수평 중앙 정렬
				.VAlign(VAlign_Center) // <--- 수직 중앙 정렬
				[
					// 2. SBorder의 자식으로 STextBlock을 넣습니다.
					SNew(STextBlock)
						.Text(FText::Format(FText::FromString(TEXT("{0}개의 애셋이 선택되었습니다.")), SelectedAssets.Num()))
				]
		];

	FSlateApplication::Get().AddWindow(CustomWindow); // 일반

}
