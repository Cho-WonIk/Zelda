
#include "ZeldaEditor.h"

#include "ZCEditorStyle.h"
#include "Utils/ZCContentBrowserSelectionMenuExtender.h"
#include "Zelda/GameData/PrimaryData/ZCActorPrimaryDataAsset.h"
#include "Window/ZCActorPrimaryDataAssetEditor.h"

#define LOCTEXT_NAMESPACE "ZeldaEditor"

class FContentBrowserSelectionMenuExtender_ZCActorPrimaryDataAsset : public FContentBrowserSelectionMenuExtender<UZCActorPrimaryDataAsset>
{
public:
	FContentBrowserSelectionMenuExtender_ZCActorPrimaryDataAsset(const FText& label, const FText& toolTip, const FName styleSetName, const FName iconName) : FContentBrowserSelectionMenuExtender(label, toolTip, styleSetName, iconName)
	{}

protected:
	virtual void Execute(TArray<UZCActorPrimaryDataAsset*> SelectedAssets) const override
	{
		for (auto* Asset : SelectedAssets)
		{
			if (!Asset) continue;

			TSharedRef<FZCActorPrimaryDataAssetEditor> ModeEditor = MakeShareable(new FZCActorPrimaryDataAssetEditor);
			ModeEditor->InitModEditor(EToolkitMode::Standalone, nullptr, Asset);

		}

		//UE_LOG(LogTemp, Warning, TEXT("오픈"));
	}
};

void FZeldaEditor::StartupModule()
{

	FZCEditorStyle::Initialize();

	ContentBrowserSelectionMenuExtenders.Add(MakeShareable(new FContentBrowserSelectionMenuExtender_ZCActorPrimaryDataAsset(
		LOCTEXT("FContentBrowserSelectionMenuExtender_ZCActorPrimaryDataAsset_Label", "ZC Mesh Shape Edit"),
		LOCTEXT("FContentBrowserSelectionMenuExtender_ZCActorPrimaryDataAsset_ToolTip", "Mesh Shape Edit Tool"),
		FZCEditorStyle::GetStyleSetName(),
		GetZCPrimayDataContextMenuIconName()
	)));

	for (const auto &Extender : ContentBrowserSelectionMenuExtenders)
	{
		if (Extender.IsValid()) Extender->Extend();
	}
}

void FZeldaEditor::ShutdownModule()
{

	ContentBrowserSelectionMenuExtenders.Empty();

	FZCEditorStyle::Shutdown();

}

FName FZeldaEditor::GetZCPrimayDataContextMenuIconName()
{
	static FName iconName("ZCActorPrimaryDataAsset_PlaceholderButtonIcon");
	return iconName;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZeldaEditor, ZeldaEditor)