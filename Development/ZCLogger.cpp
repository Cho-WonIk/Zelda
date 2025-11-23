// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/ZCLogger.h"
#include "GameplayTagContainer.h"

DEFINE_LOG_CATEGORY(ZCLog);

FString UZCLogger::ToString(const FString& Value)
{
	return Value;
}

FString UZCLogger::ToString(const FName& Value)
{
	return Value.ToString();
}

FString UZCLogger::ToString(const int32 Value)
{
	return FString::FromInt(Value);
}

FString UZCLogger::ToString(const float Value)
{
	return FString::SanitizeFloat(Value);
}

FString UZCLogger::ToString(const double Value)
{
	return FString::SanitizeFloat(Value);
}

FString UZCLogger::ToString(const bool Value)
{
	return Value ? TEXT("true") : TEXT("false");
}

FString UZCLogger::ToString(const TCHAR* Value)
{
	return FString(Value);
}

FString UZCLogger::ToString(const FString* Value)
{
	return *Value;
}

FString UZCLogger::ToString(const TWeakObjectPtr<AActor>& Actor)
{
	return Actor.IsValid() ? Actor->GetName() : TEXT("nullptr");
}

FString UZCLogger::ToString(const UObject* Object)
{
	return IsValid(Object) ? Object->GetName() : TEXT("nullptr");
}

FString UZCLogger::ToString(const FGameplayTag& Tag)
{
	return Tag.ToString();
}