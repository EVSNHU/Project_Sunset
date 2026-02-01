#include "NeonAttributeSet.h"
#include "GameplayEffectExtension.h"

/**
 * Constructor - Initializes all attributes to their default values.
 */
UNeonAttributeSet::UNeonAttributeSet()
{
	// Initialize Health system
	InitHealth(100.f);
	InitMaxHealth(100.f);
	
	// Initialize Neon (mana/style) system
	InitNeon(100.f);
	InitMaxNeon(100.f);
	
	// Initialize Stamina system
	InitStamina(100.f);
	InitMaxStamina(100.f);
	
	// Initialize Ultimate system (starts empty)
	InitUltimateCharge(0.0f);
	InitMaxUltimateCharge(50.0f);
}

/**
 * Pre-processes attribute changes before they're applied.
 * Ensures max values never go below safe minimums.
 */
void UNeonAttributeSet::PreAttributeChange(
	const FGameplayAttribute& Attribute,
	float& NewValue
)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Prevent MaxHealth from being reduced below 1 (would cause issues)
	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}

	// Prevent MaxNeon from going negative
	if (Attribute == GetMaxNeonAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}

	// Prevent MaxStamina from going negative
	if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

/**
 * Post-processes Gameplay Effect executions.
 * Handles clamping current values to max values and broadcasts damage events.
 */
void UNeonAttributeSet::PostGameplayEffectExecute(
	const FGameplayEffectModCallbackData& Data
)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Warning, TEXT("PostGameplayEffectExecute called on actor: %s"), 
		*GetOwningActor()->GetName());

	// ========================================
	// Health Modification Handling
	// ========================================
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("Health attribute was modified!"));
		UE_LOG(LogTemp, Warning, TEXT("Magnitude: %.1f (negative = damage)"), 
			Data.EvaluatedData.Magnitude);
		
		// Clamp health between 0 and max
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		// Check if damage was dealt (negative magnitude indicates damage)
		if (Data.EvaluatedData.Magnitude < 0.0f)
		{
			float DamageAmount = FMath::Abs(Data.EvaluatedData.Magnitude);
			
			UE_LOG(LogTemp, Error, TEXT("=== DAMAGE DETECTED ==="));
			UE_LOG(LogTemp, Error, TEXT("Damage Amount: %.1f to actor: %s"), 
				DamageAmount, *GetOwningActor()->GetName());
			UE_LOG(LogTemp, Error, TEXT("Delegate IsBound: %s"), 
				OnDamageTaken.IsBound() ? TEXT("TRUE") : TEXT("FALSE"));
			
			// Broadcast damage event (characters bind to this for reactions)
			OnDamageTaken.Broadcast(DamageAmount, GetOwningActor());
			UE_LOG(LogTemp, Error, TEXT("Broadcast called!"));
		}
		else
		{
			// Positive magnitude = healing
			UE_LOG(LogTemp, Log, TEXT("Healing detected (positive magnitude): %.1f"), 
				Data.EvaluatedData.Magnitude);
		}
	}

	// ========================================
	// Neon Modification Handling
	// ========================================
	if (Data.EvaluatedData.Attribute == GetNeonAttribute())
	{
		// Clamp Neon between 0 and max
		SetNeon(FMath::Clamp(GetNeon(), 0.0f, GetMaxNeon()));
	}

	// ========================================
	// Stamina Modification Handling
	// ========================================
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Clamp Stamina between 0 and max
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
		UE_LOG(LogTemp, Log, TEXT("Stamina changed: %.1f / %.1f"), 
			GetStamina(), GetMaxStamina());
	}

	// ========================================
	// Ultimate Charge Modification Handling
	// ========================================
	if (Data.EvaluatedData.Attribute == GetUltimateChargeAttribute())
	{
		// Clamp Ultimate Charge between 0 and max
		SetUltimateCharge(FMath::Clamp(GetUltimateCharge(), 0.0f, GetMaxUltimateCharge()));
		UE_LOG(LogTemp, Log, TEXT("Ultimate Charge: %.0f / %.0f"), 
			GetUltimateCharge(), GetMaxUltimateCharge());
	}
}