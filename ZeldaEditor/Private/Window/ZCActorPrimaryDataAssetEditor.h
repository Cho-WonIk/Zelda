#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"

class SMeshPreviewViewport;
class UZCActorPrimaryDataAsset;

class FZCActorPrimaryDataAssetEditor : public FAssetEditorToolkit, public FGCObject, public FEditorUndoClient, public FNotifyHook
{
public:
	FZCActorPrimaryDataAssetEditor();
	virtual ~FZCActorPrimaryDataAssetEditor();

	void InitModEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UZCActorPrimaryDataAsset* PrimaryAssetToMod);

	// FAssetEditorToolkit Interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	// FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FZCActorPrimaryDataAssetEditor"); }

	// FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	// FNotifyHook Interface
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override {}
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;


private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	void OnAssetPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
	void RefreshViewport();

private:
	static const FName ViewportTabId;
	static const FName DetailsTabId;

	TObjectPtr<class UZCActorPrimaryDataAsset> PrimaryAsset;

	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<SMeshPreviewViewport> ViewportWidget;
};

// ZC Mesh Viewport Client, PDI방식 Shape렌더링
class FZCMeshViewportClient : public FEditorViewportClient
{
public:
	FZCMeshViewportClient(FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget);

	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	void SetShapeData(const struct FZCShape* InShape);
	void SetMeshOrigin(const FVector& InOrigin);

private:
	const FZCShape* ShapeData;
	FVector MeshOrigin;
};

// 통합 메시 프리뷰 뷰포트
class SMeshPreviewViewport : public SEditorViewport, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SMeshPreviewViewport) {}
		SLATE_ARGUMENT(TWeakPtr<FZCActorPrimaryDataAssetEditor>, EditorToolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 메시 에셋 설정 (StaticMesh 또는 SkeletalMesh)
	void SetMeshAsset(UStreamableRenderAsset* InAsset);
	void ClearMesh();

	// Shape 시각화 업데이트
	void UpdateShapeVisualization(const struct FZCShape* InShape);

	// FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("SMeshPreviewViewport"); }

protected:
	// SEditorViewport Interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

private:
	void FocusCameraOnMesh();
	void CreatePreviewComponent(UStreamableRenderAsset* InAsset);

private:
	TWeakPtr<FZCActorPrimaryDataAssetEditor> EditorToolkit;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	TSharedPtr<FZCMeshViewportClient> ViewportClient;

	// 현재 표시중인 메시 컴포넌트 (StaticMeshComponent 또는 SkeletalMeshComponent)
	TWeakObjectPtr<UMeshComponent> CurrentMeshComponent;
};
