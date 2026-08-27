
#pragma once

#include "CoreMinimal.h"

namespace Zelda
{
	namespace Channel
	{

		static constexpr ECollisionChannel Damage				= ECC_GameTraceChannel1;
		static constexpr ECollisionChannel CameraToTarget		= ECC_GameTraceChannel2;
		static constexpr ECollisionChannel CameraAdjust			= ECC_GameTraceChannel3;
		static constexpr ECollisionChannel Climbable			= ECC_GameTraceChannel4;
		static constexpr ECollisionChannel Ultrahand			= ECC_GameTraceChannel5;

	}

	namespace Object
	{
		static constexpr ECollisionChannel Interaction			= ECC_GameTraceChannel6;
	}

	namespace Profile
	{
		static const FName Player								= TEXT("Player");
		static const FName HighlightArea						= TEXT("HighlightArea");
		static const FName InteractionTrigger					= TEXT("InteractionTrigger");

		static const FName Item									= TEXT("Item");
		static const FName Weapon								= TEXT("Weapon");
		static const FName Shield								= TEXT("Shield");

		static const FName Gear									= TEXT("Gear");
	}
}