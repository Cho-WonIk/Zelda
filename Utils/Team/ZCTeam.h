
#pragma once

#include "GenericTeamAgentInterface.h"

namespace Zelda
{
	namespace TeamID
	{
		const FGenericTeamId Player(0);			// 플레이어

		const FGenericTeamId FriendlyNPC(1);	// 플레이어와 아군

		const FGenericTeamId Neutral(255);		// 중립

		// 나머지 몬스터들
	}
}

struct ZCTeamUtils
{
public:
	static ETeamAttitude::Type GetTeamAttitude(const FGenericTeamId& A, const FGenericTeamId& B)
	{
		// 둘 중 하나라도 중립이면 항상 중립
		if (A == Zelda::TeamID::Neutral || B == Zelda::TeamID::Neutral)
		{
			return ETeamAttitude::Neutral;
		}

		// Player <-> FriendlyNPC 관계는 아군
		if ((A == Zelda::TeamID::Player && B == Zelda::TeamID::FriendlyNPC) ||
			(A == Zelda::TeamID::FriendlyNPC && B == Zelda::TeamID::Player))
		{
			return ETeamAttitude::Friendly;
		}

		// 동일 팀이면 아군
		if (A == B)
		{
			return ETeamAttitude::Friendly;
		}

		// 기본은 적대
		return ETeamAttitude::Hostile;
	}
};