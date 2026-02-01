#include "NeonDamageExecCalculation.h"
#include "NeonAttributeSet.h"
#include "AbilitySystemComponent.h"

/**
 * Static struct that defines which attributes this calculation captures.
 * We capture Health because we're going to reduce it (deal damage).
 */
struct NeonDamageStatics
{
	// Macro that declares we want to capture the Health attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	NeonDamageStatics()
	{
		// Define how we capture Health:
		// - From UNeonAttributeSet
		// - The Health attribute specifically
		// - From the Target (not the Source/attacker)
		// - false = don't snapshot (use current value)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNeonAttributeSet, Health, Target, false);
	}
};

/**
 * Returns a singleton instance of our damage statics.
 * This is created once and reused for all damage calculations.
 */
static const NeonDamageStatics& DamageStatics()
{
	static NeonDamageStatics DStatics;
	return DStatics;
}

/**
 * Constructor - Registers which attributes we need for our calculation.
 */
UNeonDamageExecCalculation::UNeonDamageExecCalculation()
{
	// Tell the system we need access to the target's Health attribute
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
}

/**
 * Executes the damage calculation with combo multiplier logic.
 * 
 * Combo System:
 * 1. Check if target has "Status.Corrupted" tag
 * 2. Check if damage has "Damage.Type.Neon" tag
 * 3. If both true: multiply damage by 2.5x
 * 4. Apply final damage to target's Health
 */
void UNeonDamageExecCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Get the target's Ability System Component (needed to check tags)
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	// Get the Gameplay Effect spec (contains tags and damage values)
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	// ========================================
	// Step 1: Get Gameplay Tags
	// ========================================
	
	// Tag indicating target is corrupted (debuff status)
	FGameplayTag StatusCorrupted = FGameplayTag::RequestGameplayTag(FName("Status.Corrupted"));
	
	// Tag indicating this damage is Neon-type
	FGameplayTag DamageNeon = FGameplayTag::RequestGameplayTag(FName("Damage.Type.Neon"));
	
	// Tag used to pass damage value from Blueprint
	FGameplayTag DataDamage = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

	// ========================================
	// Step 2: Check Combo Conditions
	// ========================================
	
	// Does target have the Corrupted status effect?
	bool bIsTargetCorrupted = false;
	if (TargetASC)
	{
		bIsTargetCorrupted = TargetASC->HasMatchingGameplayTag(StatusCorrupted);
	}
	
	// Is this Neon-type damage?
	bool bIsNeonDamage = AssetTags.HasTag(DamageNeon);

	// ========================================
	// Step 3: Get Base Damage Value
	// ========================================
	
	// Retrieve damage value sent by Blueprint using SetByCaller
	// Parameters: (Tag to check, bWarnIfNotFound, DefaultValue)
	float BaseDamage = Spec.GetSetByCallerMagnitude(DataDamage, false, -1.0f);

	// Fallback if Blueprint didn't provide damage value
	if (BaseDamage == -1.0f)
	{
		BaseDamage = 10.0f; // Safe default value
		UE_LOG(LogTemp, Error, 
			TEXT("NeonDamageExec: No Damage Value Found! Defaulting to 10. Check your Gameplay Ability."));
	}

	// ========================================
	// Step 4: Apply Combo Multiplier
	// ========================================
	
	if (bIsTargetCorrupted && bIsNeonDamage)
	{
		// COMBO TRIGGERED! Neon damage against corrupted target = 2.5x damage
		UE_LOG(LogTemp, Warning, TEXT(">>> COMBO TRIGGERED! Neon vs Corrupted = 2.5x Damage <<<"));
		BaseDamage *= 2.5f;
	}
	else
	{
		// No combo - log why for debugging
		UE_LOG(LogTemp, Display, TEXT("No Combo. Corrupted: %s | Neon: %s"), 
			bIsTargetCorrupted ? TEXT("YES") : TEXT("NO"), 
			bIsNeonDamage ? TEXT("YES") : TEXT("NO"));
	}

	// ========================================
	// Step 5: Apply Final Damage
	// ========================================
	
	if (BaseDamage > 0.f)
	{
		// Add a modifier that subtracts health (negative = damage)
		// This goes through PostGameplayEffectExecute where it triggers damage delegate
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				DamageStatics().HealthProperty, 
				EGameplayModOp::Additive, 
				-BaseDamage // Negative to reduce health
			)
		);
	}
}