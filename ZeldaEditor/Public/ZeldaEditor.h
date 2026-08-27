
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IContentBrowserSelectionMenuExtender;

class FZeldaEditor : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FName GetZCPrimayDataContextMenuIconName();

private:
	TArray<TSharedPtr<IContentBrowserSelectionMenuExtender>> ContentBrowserSelectionMenuExtenders;
};