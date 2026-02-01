#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "NeonDamageExecCalculation.generated.h"

/**
 * Custom damage calculation that applies systemic multipliers based on gameplay tags.
 * Specifically handles the "Neon + Corruption = Bonus Damage" combo mechanic.
 * 
 * How it works:
 * - If target has "Status.Corrupted" tag AND damage has "Damage.Type.Neon" tag
 * - Then damage is multiplied by 2.5x
 * 
 * This allows for strategic gameplay where players corrupt enemies first,
 * then follow up with Neon attacks for massive damage.
 */
UCLASS()
class PROJECT_SUNSET_API UNeonDamageExecCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UNeonDamageExecCalculation();

	/**
	 * Main execution function that calculates final damage.
	 * Checks for tag combos and applies appropriate multipliers.
	 * 
	 * @param ExecutionParams - Input parameters including source/target info and tags
	 * @param OutExecutionOutput - Output where final damage modifiers are added
	 */
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
};