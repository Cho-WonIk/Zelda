
#pragma once

#include "CoreMinimal.h"
#include <type_traits>
#include "GameData/Enum/ZCUIEvent.h"
#include "GameData/Enum/ZCWidget.h"

template<typename TEnum>
struct FTEvent
{
	static_assert(std::is_enum_v<TEnum>, "TEnum must be an enum type.");

	using UnderlyingType = std::underlying_type_t<TEnum>;

protected:
	TEnum ButtonFlagBit = static_cast<TEnum>(0);

public:
	TEnum GetPriority() const
	{
		UnderlyingType CurrentValue = static_cast<UnderlyingType>(ButtonFlagBit);

		if (CurrentValue == 0) return static_cast<TEnum>(0);

		uint32 Index = FMath::FloorLog2(CurrentValue);

		UnderlyingType HighestFlag = (static_cast<UnderlyingType>(1) << Index);

		return static_cast<TEnum>(HighestFlag);
	}

	bool HasFlag(TEnum FlagToCheck) const
	{
		return (ButtonFlagBit & FlagToCheck) != static_cast<TEnum>(0);
	}
	
	bool IsEmpty() const
	{
		return ButtonFlagBit == static_cast<TEnum>(0);
	}

	void Add(TEnum FlagToAdd)
	{
		ButtonFlagBit |= FlagToAdd;
	}

	void Remove(TEnum FlagToRemove)
	{
		ButtonFlagBit &= ~FlagToRemove;
	}

	void Update(TEnum FlagToUpdate)
	{
		Reset();
		Add(FlagToUpdate);
	}

	void Reset()
	{
		ButtonFlagBit = static_cast<TEnum>(0);
	}
};

struct FZCFaceButtonContext
{
	EFaceButtonType FaceButton;
	FText Label;
};
