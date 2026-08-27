
#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"

class IContentBrowserSelectionMenuExtender
{
public:
	virtual void Extend() = 0;
};

template<class T>
class FContentBrowserSelectionMenuExtender : public IContentBrowserSelectionMenuExtender, public TSharedFromThis<FContentBrowserSelectionMenuExtender<T>>
{
public:
	FContentBrowserSelectionMenuExtender(const FText& label, const FText& toolTip, const FName styleSetName, const FName iconName)
		: Label(label), ToolTip(toolTip), StyleSetName(styleSetName), IconName(iconName)
	{
	}

	virtual ~FContentBrowserSelectionMenuExtender() = default;

	// 우클릭 메뉴에 등록
	virtual void Extend() override
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateSP(this, &FContentBrowserSelectionMenuExtender::CreateExtender));
	}

protected:
	// 순수 가상함수, 실제 실행할 에디터 창 바인드
	virtual void Execute(TArray<T*> SelectedAssets) const = 0;

private:
	TSharedRef<FExtender> CreateExtender(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender = MakeShared<FExtender>();

		Extender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateSP(this, &FContentBrowserSelectionMenuExtender::AddMenuExtension, SelectedAssets)
		);

		return Extender;
	}

	// 사용자가 선택한 파일에 에디터 창을 열 수 있는 지 체크, 하나의 파일이라도 충족하지 못하면 실패
	void AddMenuExtension(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		TArray<T*> typedSelectedAssets;

		for (const FAssetData& SelectedAsset : SelectedAssets)
		{
			if (!SelectedAsset.GetClass()->IsChildOf(T::StaticClass())) return;

			typedSelectedAssets.Add(static_cast<T*>(SelectedAsset.GetAsset()));
		}

		if (typedSelectedAssets.Num() == 0) return;

		// 모든 조건을 만족하므로 Execute에 바인드된 에디터 창을 불러올 수 있게 표시
		MenuBuilder.AddMenuEntry(
			Label,
			ToolTip,
			FSlateIcon(StyleSetName, IconName),
			FUIAction(FExecuteAction::CreateSP(this, &FContentBrowserSelectionMenuExtender::Execute, typedSelectedAssets), FCanExecuteAction())
		);
	}

protected:

	const FText Label;
	const FText ToolTip;
	const FName StyleSetName;
	const FName IconName;
};