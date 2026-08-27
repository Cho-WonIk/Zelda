
#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

// 지금은 사용 안함
class FZCAssetTypeActions_ZCActorPrimaryDataAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return FText::FromString(TEXT("ZC Actor Primary Data Asset")); }
	virtual FColor GetTypeColor() const override { return FColor::Green; }
	virtual UClass* GetSupportedClass() const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	virtual uint32 GetCategories() override;

private:
	void ExecuteAction(TArray<TWeakObjectPtr<UObject>> InObjects);
};