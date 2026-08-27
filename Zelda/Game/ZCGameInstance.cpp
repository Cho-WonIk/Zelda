// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZCGameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCGameInstance)

void UZCGameInstance::Init()
{
	Super::Init();
}

void UZCGameInstance::Shutdown()
{
	Super::Shutdown();
}

#if WITH_EDITOR
void UZCGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* ChangedProperty = PropertyChangedEvent.Property;
	if (!ChangedProperty) return;

	const FName PropertyName = ChangedProperty->GetFName();

	// 어떤 프로퍼티가 바뀌었는지 체크
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZCGameInstance, ChemistryElementTable))
	{

	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UZCGameInstance, ChemistryMaterialTable))
	{

	}

}
#endif // WITH_EDITOR

