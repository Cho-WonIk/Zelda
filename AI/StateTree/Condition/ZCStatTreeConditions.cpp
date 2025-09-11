
#include "AI/StateTree/Condition/ZCStatTreeConditions.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeNodeDescriptionHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZCStatTreeConditions)

#define LOCTEXT_NAMESPACE "StateTree"

namespace Zelda::StateTree::Conditions
{
	template<typename T>
	bool CompareNumbers(const T Left, const T Right, const EGenericAICheck Operator)
	{
		switch (Operator)
		{
		case EGenericAICheck::Equal:
			return Left == Right;
			break;
		case EGenericAICheck::NotEqual:
			return Left != Right;
			break;
		case EGenericAICheck::Less:
			return Left < Right;
			break;
		case EGenericAICheck::LessOrEqual:
			return Left <= Right;
			break;
		case EGenericAICheck::Greater:
			return Left > Right;
			break;
		case EGenericAICheck::GreaterOrEqual:
			return Left >= Right;
			break;
		default:
			ensureMsgf(false, TEXT("Unhandled operator %d"), Operator);
			return false;
			break;
		}
	}
}

bool FZCCompareActorDistanceCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.SourceActor || !InstanceData.TargetActor)
	{
		SET_NODE_CUSTOM_TRACE_TEXT(Context, Override, TEXT("Source or Target actor is not set."));
		return false;
	}

	const FVector::FReal Left = FVector::DistSquared(InstanceData.SourceActor->GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
	const FVector::FReal Right = FMath::Square(InstanceData.Distance);

	const bool bResult = Zelda::StateTree::Conditions::CompareNumbers<FVector::FReal>(Left, Right, Operator);

	SET_NODE_CUSTOM_TRACE_TEXT(Context, Override, TEXT("%sDistance %s %s %s (from [%s] to [%s])"),
		*UE::StateTree::DescHelpers::GetInvertText(bInvert, EStateTreeNodeFormatting::Text).ToString(),
		*LexToString(Left),
		*UE::StateTree::DescHelpers::GetOperatorText(Operator, EStateTreeNodeFormatting::Text).ToString(),
		*LexToString(Right),
		*InstanceData.SourceActor->GetActorLocation().ToString(),
		*InstanceData.TargetActor->GetActorLocation().ToString());

	return bResult ^ bInvert;
}

#if WITH_EDITOR
FText FZCCompareActorDistanceCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();

	check(InstanceData);

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 1;
	Options.MaximumFractionalDigits = 3;

	FText SourceActorText = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, SourceActor)), Formatting);
	if (SourceActorText.IsEmpty())
	{
		SourceActorText = InstanceData->SourceActor ? FText::FromString(InstanceData->SourceActor->GetName()) : FText::GetEmpty();
	}

	FText TargetActorText = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, TargetActor)), Formatting);
	if (TargetActorText.IsEmpty())
	{
		TargetActorText = InstanceData->TargetActor ? FText::FromString(InstanceData->TargetActor->GetName()) : FText::GetEmpty();
	}

	FText DistanceValue = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, Distance)), Formatting);
	if (DistanceValue.IsEmpty())
	{
		DistanceValue = FText::AsNumber(InstanceData->Distance, &Options);
	}

	const FText OperatorText = UE::StateTree::DescHelpers::GetOperatorText(Operator, Formatting);
	const FText InvertText = UE::StateTree::DescHelpers::GetInvertText(bInvert, Formatting);

	const FText Format = LOCTEXT("CompareActorDistanceSimple", "{EmptyOrNot}{Source}와 {Target} 사이 거리 {Op} {Distance}");

	return FText::FormatNamed(Format,
		TEXT("EmptyOrNot"), InvertText,
		TEXT("Source"), SourceActorText,
		TEXT("Target"), TargetActorText,
		TEXT("Op"), OperatorText,
		TEXT("Distance"), DistanceValue);
}
#endif // WITH_EDITOR
