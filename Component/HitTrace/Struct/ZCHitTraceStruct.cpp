

#include "ZCHitTraceStruct.h"
#include "Engine/EngineTypes.h"
#include "Physics/ZCCollision.h"

const FZCHitTraceStruct FZCHitTraceStruct::ObjectDefault = [] {
    FZCHitTraceStruct HitStruct;

    HitStruct.TraceType = EHitTraceType::ObjectType;
    HitStruct.BeginSocket = ZCHitTraceSockets::Top;
    HitStruct.EndSocket = ZCHitTraceSockets::Bottom;
    HitStruct.Radius = 15.0f;
    HitStruct.TraceComplex = false;
    HitStruct.bIgnoreSelf = true;

	HitStruct.bAllowMultipleHit = false;
	HitStruct.HitInterval = 0.0f;

    HitStruct.DrawDebugType = EDrawDebugTrace::None;

    HitStruct.ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    return HitStruct;
    }();

const FZCHitTraceStruct FZCHitTraceStruct::ChannelDefault = [] {
    FZCHitTraceStruct HitStruct;

    HitStruct.TraceType = EHitTraceType::Channel;
    HitStruct.BeginSocket = ZCHitTraceSockets::Top;
    HitStruct.EndSocket = ZCHitTraceSockets::Bottom;
    HitStruct.Radius = 15.0f;
    HitStruct.TraceComplex = false;
    HitStruct.bIgnoreSelf = true;

    HitStruct.bAllowMultipleHit = false;
    HitStruct.HitInterval = 0.0f;

    HitStruct.DrawDebugType = EDrawDebugTrace::None;

    HitStruct.TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);

    return HitStruct;
    }();

const FZCHitTraceStruct FZCHitTraceStruct::ProfileDefault = [] {
    FZCHitTraceStruct HitStruct;

    HitStruct.TraceType = EHitTraceType::ProfileName;
    HitStruct.BeginSocket = ZCHitTraceSockets::Top;
    HitStruct.EndSocket = ZCHitTraceSockets::Bottom;
    HitStruct.Radius = 15.0f;
    HitStruct.TraceComplex = false;
    HitStruct.bIgnoreSelf = true;

    HitStruct.bAllowMultipleHit = false;
    HitStruct.HitInterval = 0.0f;

    HitStruct.DrawDebugType = EDrawDebugTrace::None;

    HitStruct.ProfileName = FName(TEXT("Pawn"));

    return HitStruct;
    }();
