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
	static FORCEINLINE FString ToString(const FString& Value)			{ return Value; }
	static FORCEINLINE FString ToString(const FName& Value)				{ return Value.ToString(); }
	static FORCEINLINE FString ToString(const int32 Value)				{ return FString::FromInt(Value); }
	static FORCEINLINE FString ToString(const float Value)				{ return FString::SanitizeFloat(Value); }
	static FORCEINLINE FString ToString(const double Value)				{ return FString::SanitizeFloat(Value); }
	static FORCEINLINE FString ToString(const bool Value)				{ return Value ? TEXT("true") : TEXT("false"); }
	static FORCEINLINE FString ToString(const TCHAR* Value)				{ return FString(Value); }
	static FORCEINLINE FString ToString(const FString* Value)			{ return Value ? *Value : TEXT("nullptr"); }
	static FORCEINLINE FString ToString(const UObject* Object)			{ return IsValid(Object) ? Object->GetName() : TEXT("nullptr"); }

	static FORCEINLINE FString ToString(const void* Ptr)				{ return Ptr ? FString::Printf(TEXT("0x%p"), Ptr) : TEXT("nullptr"); }

	template<typename T>
	static typename TEnableIf<TIsArithmetic<T>::Value, FString>::Type ToString(T Value)
	{
		return LexToString(Value);
	}

	template <typename T>
	static typename TEnableIf<TIsClass<T>::Value, FString>::Type ToString(const T& Value)
	{
		return Value.ToString();
	}

	template<typename T>
	static FString ToString(const TObjectPtr<T>& Value)
	{
		return ToString(Value.Get());
	}

	template<typename T>
	static FString ToString(const TWeakObjectPtr<T>& Value)
	{
		return ToString(Value.Get());
	}

	template<typename T>
	static FString ToString(const TSoftObjectPtr<T>& Value)
	{
		return Value.ToString();
	}

	template<typename T>
	static FString ToString(T EnumValue, typename TEnableIf<TIsEnum<T>::Value>::Type* = nullptr)
	{
		return *UEnum::GetValueAsString(EnumValue);
	}
};
