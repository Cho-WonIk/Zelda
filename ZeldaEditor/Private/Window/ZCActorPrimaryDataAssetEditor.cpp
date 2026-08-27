
#include "ZCActorPrimaryDataAssetEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "SceneManagement.h"
#include "UnrealWidget.h"


#include "Zelda/Physics/ZCShape.h"
#include "Zelda/GameData/PrimaryData/ZCActorPrimaryDataAsset.h"
#include "Settings/ZCActorPrimaryDataAssetEditorSettings.h"


#define LOCTEXT_NAMESPACE "ZCActorPrimaryDataAssetEditor"

const FName FZCActorPrimaryDataAssetEditor::ViewportTabId(TEXT("ZCActorPrimaryDataAssetEditor_Viewport"));
const FName FZCActorPrimaryDataAssetEditor::DetailsTabId(TEXT("ZCActorPrimaryDataAssetEditor_Details"));

// ============================================================================
// FZCActorPrimaryDataAssetEditor
// ============================================================================

FZCActorPrimaryDataAssetEditor::FZCActorPrimaryDataAssetEditor()
{
}

FZCActorPrimaryDataAssetEditor::~FZCActorPrimaryDataAssetEditor()
{
}

void FZCActorPrimaryDataAssetEditor::InitModEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UZCActorPrimaryDataAsset* PrimaryAssetToMod)
{
	PrimaryAsset = PrimaryAssetToMod;

	// Details View 생성
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(PrimaryAsset);
	//DetailsView->SetObjects(TArray<UObject*>(PrimaryAssetToMod));
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FZCActorPrimaryDataAssetEditor::OnAssetPropertyChanged);

	// 레이아웃 정의
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_ZCActorPrimaryDataAssetEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.7f)
					->AddTab(ViewportTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, FName("ZCActorPrimaryDataAssetEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, PrimaryAssetToMod);

	RefreshViewport();
}

void FZCActorPrimaryDataAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_ZCMeshEditor", "ZC Mesh Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ViewportTabId, FOnSpawnTab::CreateSP(this, &FZCActorPrimaryDataAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FZCActorPrimaryDataAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FZCActorPrimaryDataAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(ViewportTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
}

TSharedRef<SDockTab> FZCActorPrimaryDataAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	ViewportWidget = SNew(SMeshPreviewViewport)
		.EditorToolkit(SharedThis(this));

	return SNew(SDockTab).Label(LOCTEXT("ViewportTab_Title", "Mesh Viewport"))[ViewportWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FZCActorPrimaryDataAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("DetailsTab_Title", "Details"))[DetailsView.ToSharedRef()];
}

void FZCActorPrimaryDataAssetEditor::OnAssetPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// Mesh 프로퍼티가 변경되었을 때 뷰포트 갱신
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UZCActorPrimaryDataAsset, Mesh))
	{
		RefreshViewport();
	}
	// Shape 프로퍼티가 변경되었을 때 Shape 시각화 갱신
	else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UZCActorPrimaryDataAsset, Shape))
	{
		if (ViewportWidget.IsValid() && PrimaryAsset != nullptr && PrimaryAsset.Get())
		{
			ViewportWidget->UpdateShapeVisualization(&PrimaryAsset->Shape);
		}
	}
}

void FZCActorPrimaryDataAssetEditor::RefreshViewport()
{
	if (!ViewportWidget.IsValid() || PrimaryAsset == nullptr) return;

	UZCActorPrimaryDataAsset* Asset = PrimaryAsset.Get();
	if (!Asset) return;

	// TSoftObjectPtr를 로드
	UStreamableRenderAsset* MeshAsset = Asset->Mesh.LoadSynchronous();
	ViewportWidget->SetMeshAsset(MeshAsset);

	// Shape도 함께 업데이트
	ViewportWidget->UpdateShapeVisualization(&Asset->Shape);
}

FName FZCActorPrimaryDataAssetEditor::GetToolkitFName() const
{
	return FName("ZCActorPrimaryDataAssetEditor");
}

FText FZCActorPrimaryDataAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "ZC Mesh Editor");
}

FString FZCActorPrimaryDataAssetEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "ZC Mesh ").ToString();
}

FLinearColor FZCActorPrimaryDataAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

void FZCActorPrimaryDataAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrimaryAsset);
}

void FZCActorPrimaryDataAssetEditor::PostUndo(bool bSuccess)
{
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(PrimaryAsset);
	}
	RefreshViewport();
}

void FZCActorPrimaryDataAssetEditor::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

void FZCActorPrimaryDataAssetEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	// Mesh 프로퍼티가 변경되었을 때 뷰포트 갱신
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UZCActorPrimaryDataAsset, Mesh))
	{
		RefreshViewport();
	}
	// Shape 프로퍼티가 변경되었을 때 Shape 시각화 갱신
	else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UZCActorPrimaryDataAsset, Shape))
	{
		if (ViewportWidget.IsValid() && PrimaryAsset != nullptr && PrimaryAsset.Get())
		{
			ViewportWidget->UpdateShapeVisualization(&PrimaryAsset->Shape);
		}
	}
}

// ============================================================================
// FZCMeshViewportClient
// ============================================================================

FZCMeshViewportClient::FZCMeshViewportClient(FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget)
	: FEditorViewportClient(nullptr, InPreviewScene, InEditorViewportWidget)
	, ShapeData(nullptr)
	, MeshOrigin(FVector::ZeroVector)
{
	SetViewMode(VMI_Lit);
	EngineShowFlags.SetSelectionOutline(false);
	EngineShowFlags.SetGrid(false);
	SetRealtime(true);
	bSetListenerPosition = false;

	// 초기 카메라 위치
	SetViewLocation(FVector(-200, 200, 150));
	SetLookAtLocation(FVector::ZeroVector);
}

void FZCMeshViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);
	if (!ShapeData || !PDI) return;

	const auto* Set = GetDefault<UZCActorPrimaryDataAssetEditorSettings>();
	const FTransform Transform(ShapeData->Rotation, MeshOrigin);
	const FVector Pos = Transform.TransformPosition(ShapeData->Start);
	const FMatrix Matrix = FTransform(ShapeData->Rotation, Pos).ToMatrixNoScale();

	switch (ShapeData->Type)
	{
	case EZCShapeType::Box:
		DrawWireBox(PDI, Matrix, FBox(-ShapeData->Info, ShapeData->Info), Set->ShapeColor, Set->DepthPriority, Set->ShapeThickness);
		break;
	case EZCShapeType::Sphere:
		DrawWireSphere(PDI, Pos, Set->ShapeColor, ShapeData->Info.X, 32, Set->DepthPriority, Set->ShapeThickness);
		break;
	case EZCShapeType::Capsule:
		DrawWireCapsule(PDI, Pos, Transform.GetUnitAxis(EAxis::Z), Transform.GetUnitAxis(EAxis::X), Transform.GetUnitAxis(EAxis::Y), Set->ShapeColor, ShapeData->Info.X, ShapeData->Info.Y, 16, Set->DepthPriority, Set->ShapeThickness);
		break;
	case EZCShapeType::Line:
	{
		FVector Dir = Transform.GetUnitAxis(EAxis::X);
		FVector End = Pos + Dir * ShapeData->Info.X;
		PDI->DrawLine(Pos, End, Set->ShapeColor, Set->DepthPriority, Set->ShapeThickness);
		if (Set->bShowEndpointSpheres)
		{
			DrawWireSphere(PDI, Pos, Set->ShapeColor, Set->LineEndpointSphereSize, 8, Set->DepthPriority);
			DrawWireSphere(PDI, End, Set->ShapeColor, Set->LineEndpointSphereSize, 8, Set->DepthPriority);
		}
		break;
	}
	}
}

void FZCMeshViewportClient::SetShapeData(const FZCShape* InShape)
{
	ShapeData = InShape;
	Invalidate();
}

void FZCMeshViewportClient::SetMeshOrigin(const FVector& InOrigin)
{
	MeshOrigin = InOrigin;
	Invalidate();
}

// ============================================================================
// SMeshPreviewViewport
// ============================================================================

void SMeshPreviewViewport::Construct(const FArguments& InArgs)
{
	EditorToolkit = InArgs._EditorToolkit;

	PreviewScene = MakeShared<FAdvancedPreviewScene>(FPreviewScene::ConstructionValues());
	PreviewScene->SetFloorVisibility(false);

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SMeshPreviewViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FZCMeshViewportClient>(PreviewScene.Get(), SharedThis(this));
	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SMeshPreviewViewport::MakeViewportToolbar()
{
	// 필요시 커스텀 툴바 추가
	return SNew(SBox);
}

void SMeshPreviewViewport::SetMeshAsset(UStreamableRenderAsset* InAsset)
{
	ClearMesh();

	if (InAsset)
	{
		CreatePreviewComponent(InAsset);
		FocusCameraOnMesh();

		// 메시 원점 업데이트
		if (ViewportClient.IsValid())
		{
			FVector Origin = CurrentMeshComponent.IsValid() ? CurrentMeshComponent->GetComponentLocation() : FVector::ZeroVector;
			ViewportClient->SetMeshOrigin(Origin);
		}
	}

	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate();
	}
}

void SMeshPreviewViewport::ClearMesh()
{
	if (CurrentMeshComponent.IsValid() && PreviewScene.IsValid())
	{
		PreviewScene->RemoveComponent(CurrentMeshComponent.Get());
		CurrentMeshComponent->DestroyComponent();
		CurrentMeshComponent = nullptr;
	}
}

void SMeshPreviewViewport::CreatePreviewComponent(UStreamableRenderAsset* InAsset)
{
	if (!InAsset || !PreviewScene.IsValid()) return;

	UMeshComponent* NewComponent = nullptr;

	// StaticMesh 처리
	if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(InAsset))
	{
		UStaticMeshComponent* SMC = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		SMC->SetStaticMesh(StaticMesh);
		SMC->SetMobility(EComponentMobility::Movable);
		NewComponent = SMC;
	}
	// SkeletalMesh 처리
	else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(InAsset))
	{
		USkeletalMeshComponent* SKC = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		SKC->SetSkeletalMesh(SkeletalMesh);
		SKC->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SKC->bPauseAnims = true;  // 애니메이션 정지
		SKC->SetUpdateAnimationInEditor(false);
		SKC->SetMobility(EComponentMobility::Movable);
		NewComponent = SKC;
	}

	if (NewComponent)
	{
		PreviewScene->AddComponent(NewComponent, FTransform::Identity);
		CurrentMeshComponent = NewComponent;
	}
}

void SMeshPreviewViewport::FocusCameraOnMesh()
{
	if (!CurrentMeshComponent.IsValid() || !ViewportClient.IsValid()) return;

	FBoxSphereBounds Bounds = CurrentMeshComponent->Bounds;
	float Radius = FMath::Max(Bounds.SphereRadius, 50.0f); // 최소 거리 보장
	FVector Center = Bounds.Origin;

	// 적절한 거리 계산
	float Distance = Radius * 2.5f;
	FVector CameraLocation = Center + FVector(-Distance, Distance * 0.7f, Distance * 0.5f);

	ViewportClient->SetViewLocation(CameraLocation);
	ViewportClient->SetLookAtLocation(Center);
}

void SMeshPreviewViewport::UpdateShapeVisualization(const FZCShape* InShape)
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->SetShapeData(InShape);

		// 메시 원점도 함께 업데이트
		FVector Origin = CurrentMeshComponent.IsValid() ? CurrentMeshComponent->GetComponentLocation() : FVector::ZeroVector;

		ViewportClient->SetMeshOrigin(Origin);
	}
}

void SMeshPreviewViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (CurrentMeshComponent.IsValid())
	{
		UMeshComponent* Component = CurrentMeshComponent.Get();
		Collector.AddReferencedObject(Component);
	}
}
#undef LOCTEXT_NAMESPACE