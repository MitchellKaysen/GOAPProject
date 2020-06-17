#include "../Public/PlannerComponent.h"
#include "../Public/PlannerAsset.h"
#include "../Public/GOAPAction.h"

void UPlannerComponent::OnTaskFinished(UGOAPAction* Action, EPlannerTaskFinishedResult::Type Result)
{
	if (Result == EPlannerTaskFinishedResult::Success)
	{
		PlanAdvance();
		RequestExecutionUpdate();
	}
	else
	{
		ClearCurrentPlan();
	}
}

void UPlannerComponent::StartPlanner(UPlannerAsset& PlannerAsset)
{
	ActionSet.Reserve(PlannerAsset.Actions.Num());
	for (auto* Action : PlannerAsset.Actions)
	{
		UGOAPAction* Copy = DuplicateObject<UGOAPAction>(Action, this);
		Copy->SetOwner(AIOwner, this);

		ActionSet.Emplace(Copy);
		
	}
	Asset = &PlannerAsset;
	BufferSize = PlannerAsset.MaxPlanSize + 1;
	PlanBuffer.Init(nullptr, BufferSize);
}

void UPlannerComponent::RunAllActions()
{
	StartNewPlan(ActionSet);
	RequestExecutionUpdate();

}

bool UPlannerComponent::IsRunningPlan() const
{
	return bPlanInProgress;
}

FString UPlannerComponent::GetDebugInfoString() const
{
	FString DebugInfo;
	DebugInfo += FString::Printf(TEXT("PlannerAsset: %s %d hmm\n"), *GetNameSafe(Asset), ActionSet.Num());

	for (auto* Action : ActionSet)
	{
		FString ActionName = Action ? Action->GetActionName() : FString(TEXT("None"));
		FString OpName = Action && Action->GetOperator() ? Action->GetOperator()->GetName() : FString(TEXT("None"));
		DebugInfo += FString::Printf(TEXT("Action: %s\n"), *ActionName);
		DebugInfo += FString::Printf(TEXT("    Op: %s\n"), *OpName);
	}
	for (uint32 Idx = PlanHead; Idx != PlanTail; Idx = (Idx + 1) % BufferSize)
	{
		UGOAPAction* Action = PlanBuffer[Idx];
		FString ActionName = Action ? Action->GetActionName() : FString(TEXT("None"));
		DebugInfo += FString::Printf(TEXT("Plan Step: %s\n"), *ActionName);
	}
	return DebugInfo;
}

void UPlannerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlanUpdateNeeded)
	{
		UpdatePlanExecution();
	}

}

void UPlannerComponent::RequestExecutionUpdate()
{
	bPlanUpdateNeeded = true;
}

void UPlannerComponent::UpdatePlanExecution()
{
	bPlanUpdateNeeded = false;

	if (!PlanReachedEnd())
	{
		UGOAPAction* NextAction = PlanBuffer[PlanHead];
		if (NextAction != nullptr)
		{
			NextAction->StartAction();
		}
	}
	else
	{
		bPlanInProgress = false;
	}
}

bool UPlannerComponent::PlanReachedEnd()
{
	return PlanHead == PlanTail;
}

bool UPlannerComponent::PlanAdvance()
{
	PlanBuffer[PlanHead] = nullptr; //clear previous action
	//increment pointer
	PlanFull = false;

	PlanHead = (PlanHead + 1) % BufferSize;

	//return whether we've reached the end of the buffer
	return (PlanHead == PlanTail);
}

void UPlannerComponent::StartNewPlan(TArray<UGOAPAction*>& Plan)
{
	ClearCurrentPlan();
	for (auto* Action : Plan)
	{
		AddAction(Action);
	}
	bPlanInProgress = true;
}

void UPlannerComponent::AddAction(UGOAPAction* Action)
{
	if (PlanFull)
	{
		return;
	}
	PlanBuffer[PlanTail] = Action;
	PlanTail = (PlanTail + 1) % BufferSize;
	if (PlanTail == PlanHead)
	{
		PlanFull = true;
	}
}

void UPlannerComponent::ClearCurrentPlan()
{
	PlanFull = false;
	while (PlanHead != PlanTail)
	{
		PlanBuffer[PlanHead] = nullptr;
		PlanHead = (PlanHead + 1) % BufferSize;
	}
	bPlanInProgress = false;
}