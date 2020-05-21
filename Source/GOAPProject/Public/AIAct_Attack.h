#pragma once
#include "CoreMinimal.h"
#include "GOAPAction.h"
#include "AIAct_Attack.generated.h"

class AAIController;
class UAnimMontage;
class UAITask_AnimMontage;

UCLASS(BlueprintType)
class GOAPPROJECT_API UAIAct_Attack : public UGOAPAction
{
	GENERATED_BODY()
protected:

	UPROPERTY()
		UAnimMontage* CachedMontage;

	UPROPERTY()
		UAITask_AnimMontage* MontageTaskHandle;


public:
	UAIAct_Attack();
	bool VerifyContext() override;
	EActionStatus StartAction() override;
	void StopAction() override;
	void AbortAction() override;
};
