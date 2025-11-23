// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;

DECLARE_LOG_CATEGORY_EXTERN(ZCLog, All, All);

class UZCLogger
{
public:
	template<typename... ArgTypes>
	static void Info(const TCHAR* Format, ArgTypes&&... Args)
	{
		UE_LOG(ZCLog, Log, TEXT("%s"), *FString::Format(Format, { ToString(Forward<ArgTypes>(Args))... }));
	}

	static void Info(const TCHAR* Format)
	{
		UE_LOG(ZCLog, Log, TEXT("%s"), Format);
	}

	template<typename... ArgTypes>
	static void Warning(const TCHAR* Format, ArgTypes&&... Args)
	{
		UE_LOG(ZCLog, Warning, TEXT("%s"), *FString::Format(Format, { ToString(Forward<ArgTypes>(Args))... }));
	}

	static void Warning(const TCHAR* Format)
	{
		UE_LOG(ZCLog, Warning, TEXT("%s"), Format);
	}

	template<typename... ArgTypes>
	static void Error(const TCHAR* Format, ArgTypes&&... Args)
	{
		UE_LOG(ZCLog, Error, TEXT("%s"), *FString::Format(Format, { ToString(Forward<ArgTypes>(Args))... }));
	}

	static void Error(const TCHAR* Format)
	{
		UE_LOG(ZCLog, Error, TEXT("%s"), Format);
	}

private:
	static FString ToString(const FString& Value);
	static FString ToString(const FName& Value);
	static FString ToString(const int32 Value);
	static FString ToString(const float Value);
	static FString ToString(const double Value);
	static FString ToString(const bool Value);
	static FString ToString(const TCHAR* Value);
	static FString ToString(const FString* Value);
	static FString ToString(const TWeakObjectPtr<AActor>& Actor);
	static FString ToString(const UObject* Object);
	static FString ToString(const FGameplayTag& Tag);

	template<typename T>
	static FString ToString(T EnumValue, typename TEnableIf<TIsEnum<T>::Value>::Type* = nullptr) { return *UEnum::GetValueAsString(EnumValue); }
};
