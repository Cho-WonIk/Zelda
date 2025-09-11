
#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
namespace Zelda
{
	// lower case로 작성
	namespace Debug
	{
		namespace LockOn
		{
			inline constexpr const TCHAR* DrawDebug		= TEXT("zelda.debug.lockon");
		}
		namespace HitTrace
		{
			inline constexpr const TCHAR* DrawDebug		= TEXT("zelda.debug.hittrace");
		}
		namespace Climb
		{
			inline constexpr const TCHAR* all			= TEXT("zelda.debug.climb.all");
			inline constexpr const TCHAR* player		= TEXT("zelda.debug.climb.player");
			inline constexpr const TCHAR* hitresult		= TEXT("zelda.debug.climb.hitresult");
			inline constexpr const TCHAR* ledge			= TEXT("zelda.debug.climb.ledge");
		}

		namespace AI
		{
			inline constexpr const TCHAR* all			= TEXT("zelda.debug.ai.all");
			inline constexpr const TCHAR* perception	= TEXT("zelda.debug.ai.perception");
		}

		namespace Navigation
		{
			inline constexpr const TCHAR* path			= TEXT("zelda.debug.navigation.path");
		}

		namespace StateTree
		{
			inline constexpr const TCHAR* all			= TEXT("zelda.debug.statetree.all");
			inline constexpr const TCHAR* task			= TEXT("zelda.debug.statetree.task");
		}

		namespace State
		{
			inline constexpr const TCHAR* all			= TEXT("zelda.debug.state.all");
			inline constexpr const TCHAR* show			= TEXT("zelda.debug.state.show");
		}
	}
}
#endif
