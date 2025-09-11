// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 콜백 함수 타입 정의
using FItemOverwriteCallback = TFunction<void(int32 Index)>;

// 풀 슬롯 정보
template<typename Type>
struct ZCPoolSlot
{
public:

	TUniquePtr<Type> Item;
	FItemOverwriteCallback OverwriteCallback;

	void Reset()
	{
		Item.Reset();
		OverwriteCallback = nullptr;
	}

	bool IsValid() const { return Item.IsValid(); }
};

/*============================================================================================================================*/

// 가변 길이 구조체 풀
template<typename Type, int32 PoolSize, typename InAllocator = TInlineAllocator<PoolSize>>
class ZCGrowingItemPool
{
public:
	using ValueType = TUniquePtr<Type>;

	ZCGrowingItemPool()
	{
		Pool.Reserve(PoolSize);

		for (int32 i = 0; i < PoolSize; ++i)
		{
			Pool.Add(ValueType());
			FreeStack.Push(i);
		}
	}

	[[nodiscard]] int32 AddItem(ValueType NewItem)
	{
		int32 Index;
		if (FreeStack.Num() > 0)
		{
			Index = FreeStack.Pop();
			Pool[Index] = MoveTemp(NewItem);
			return Index;
		}
		else
		{
			return Pool.Add(MoveTemp(NewItem));
		}
	}

	[[nodiscard]] ValueType TakeItem(int32 Index)
	{
		if (!IsValid(Index)) return nullptr;

		ValueType out = MoveTemp(Pool[Index]);
		FreeStack.Push(Index);
		return out;
	}

	// 인덱스에 해당하는 아이템을 제거
	void RemoveItem(int32 Index)
	{
		if (!IsValid(Index)) return;

		Pool[Index].Reset();
		FreeStack.Push(Index);
	}

	Type* operator[](int32 Index) 
	{
		return IsValid(Index) ? Pool[Index].Get() : nullptr; 
	}

	const Type* operator[](int32 Index) const 
	{
		return IsValid(Index) ? Pool[Index].Get() : nullptr; 
	}

private:
	bool IsValid(int32 Index) const { return Pool.IsValidIndex(Index) && Pool[Index].IsValid(); }

	TArray<ValueType, InAllocator> Pool;
	TArray<int32> FreeStack;

};

/*==============================================================================================================*/

// 고정 길이 구조체 풀, TArray버전
template<typename Type, int32 PoolSize, typename InAllocator = TFixedAllocator<PoolSize>>
class ZCFixedItemPool
{
public:
	static_assert(PoolSize > 0, "PoolSize must be greater than zero");

	using ValueType = TUniquePtr<Type>;
	using SlotType = ZCPoolSlot<Type>;

	ZCFixedItemPool()
	{
		Pool.Reserve(PoolSize);
		for (int32 i = 0; i < PoolSize; ++i)
		{
			Pool.Add(SlotType());
			FreeStack.Push(i);
		}
		NextOverwrite = PoolSize - 1;
	}

	// 콜백 등록/변경
	void SetOverwriteCallback(int32 Index, FItemOverwriteCallback Callback)
	{
		if (IsValid(Index))
		{
			Pool[Index].OverwriteCallback = Callback;
		}
	}

	[[nodiscard]] int32 AddItem(ValueType Item, FItemOverwriteCallback Callback = nullptr)
	{
		int32 idx;
		if (FreeStack.Num() > 0)
		{
			idx = FreeStack.Pop(); // Stack에서 Pop
		}
		else
		{
			idx = NextOverwrite;

			if (Pool[idx].OverwriteCallback)
			{
				Pool[idx].OverwriteCallback(idx); // 덮어쓰기 시 콜백 호출
			}

			Pool[idx].Reset(); // 기존 아이템 초기화
			NextOverwrite = (NextOverwrite + 1) % PoolSize;
		}
		Pool[idx].Item = MoveTemp(Item);
		Pool[idx].OverwriteCallback = Callback;
		return idx;
	}

	[[nodiscard]] ValueType TakeItem(int32 idx)
	{
		if (!IsValid(idx)) return nullptr;
		ValueType out = MoveTemp(Pool[idx].Item);
		if (Pool[idx].OverwriteCallback)
		{
			Pool[idx].OverwriteCallback(idx); // 콜백 호출
			Pool[idx].OverwriteCallback = nullptr; // 콜백 초기화
		}

		FreeStack.Push(idx); // Stack에 Push
		return out;
	}

	void RemoveItem(int32 idx)
	{
		if (!IsValid(idx)) return;
		Pool[idx].Reset();
		FreeStack.Push(idx);
	}

	Type* operator[](int32 idx)
	{
		return IsValid(idx) ? Pool[idx].Item.Get() : nullptr;
	}

	const Type* operator[](int32 idx) const
	{
		return IsValid(idx) ? Pool[idx].Item.Get() : nullptr;
	}

private:
	bool IsValid(int32 idx) const { return Pool.IsValidIndex(idx) && Pool[idx].IsValid(); }

	TArray<SlotType, InAllocator> Pool;
	TArray<int32> FreeStack;
	int32 NextOverwrite;

};


// 고정 길이 구조체 풀, TStaticArray버전
template<typename Type, int32 PoolSize>
class ZCStaticItemPool
{
public:
	static_assert(PoolSize > 0, "PoolSize must be greater than zero");

	using ValueType = TUniquePtr<Type>;
	using SlotType = ZCPoolSlot<Type>;

	ZCStaticItemPool()
	{
		for (int32 i = 0; i < PoolSize; ++i)
		{
			FreeStack.Push(i);
		}
		NextOverwrite = PoolSize - 1;
	}

	// 콜백 등록/변경
	void SetOverwriteCallback(int32 Index, FItemOverwriteCallback Callback)
	{
		if (IsValid(Index))
		{
			Pool[Index].OverwriteCallback = Callback;
		}
	}

	// 콜백과 함께 아이템 추가
	[[nodiscard]] int32 AddItem(ValueType Item, FItemOverwriteCallback Callback = nullptr)
	{
		int32 idx;
		if (FreeStack.Num() > 0)
		{
			idx = FreeStack.Pop(); // Stack에서 Pop
		}
		else
		{
			idx = NextOverwrite;
			Pool[idx].Reset();
			NextOverwrite = (NextOverwrite + 1) % PoolSize;
		}

		Pool[idx].Item = MoveTemp(Item);
		Pool[idx].OverwriteCallback = Callback;

		return idx;
	}

	[[nodiscard]] ValueType TakeItem(int32 idx)
	{
		if (!IsValid(idx)) return nullptr;

		ValueType out = MoveTemp(Pool[idx].Item);
		Pool[idx].OverwriteCallback = nullptr;
		FreeStack.Push(idx);
		return out;
	}

	void RemoveItem(int32 idx)
	{
		if (!IsValid(idx)) return;

		Pool[idx].Reset();
		FreeStack.Push(idx);
	}

	Type* operator[](int32 idx)
	{
		return IsValid(idx) ? Pool[idx].Item.Get() : nullptr;
	}

	const Type* operator[](int32 idx) const
	{
		return IsValid(idx) ? Pool[idx].Item.Get() : nullptr;
	}

private:
	bool IsValid(int32 idx) const { return idx >= 0 && idx < PoolSize && Pool[idx].IsValid(); }

	TStaticArray<SlotType, PoolSize> Pool;
	TArray<int32> FreeStack;
	int32 NextOverwrite;
};

