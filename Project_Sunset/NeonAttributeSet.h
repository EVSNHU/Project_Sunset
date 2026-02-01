#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NeonAttributeSet.generated.h"

/**
 * Macro to generate boilerplate accessor functions for GAS attributes.
 * Creates getter, setter, and initializer functions for an attribute property.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Delegate that broadcasts when an actor takes damage.
 * Other systems can bind to this to react to damage (UI updates, AI reactions, etc.)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, AActor*, DamagedActor);

/**
 * Attribute Set containing all character stats for the game.
 * Manages Health, Neon (mana/style), Stamina, and Ultimate Charge.
 * Handles clamping values and broadcasting damage events.
 */
UCLASS()
class PROJECT_SUNSET_API UNeonAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UNeonAttributeSet();

	/** 
	 * Delegate that fires when damage is applied to this attribute set's owner.
	 * Bind to this in characters to react to damage events.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnDamageTaken OnDamageTaken;

	/**
	 * Called before a Gameplay Effect modifies an attribute value.
	 * Used to clamp or validate incoming changes before they're applied.
	 * 
	 * @param Attribute - The attribute being modified
	 * @param NewValue - Reference to the new value (can be modified here)
	 */
	virtual void PreAttributeChange(
		const FGameplayAttribute& Attribute,
		float& NewValue
	) override;

	/**
	 * Called after a Gameplay Effect has been applied and executed.
	 * Handles post-processing like clamping values and broadcasting events.
	 * 
	 * @param Data - Information about the effect execution
	 */
	virtual void PostGameplayEffectExecute(
		const FGameplayEffectModCallbackData& Data
	) override;

	// ========================================
	// Health Attributes
	// ========================================
	
	/** Current health value */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, Health);

	/** Maximum health value */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, MaxHealth);

	// ========================================
	// Neon Attributes (Mana/Style System)
	// ========================================
	
	/** Current Neon value (used for abilities/style attacks) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Neon;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, Neon);

	/** Maximum Neon value */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxNeon;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, MaxNeon);

	// ========================================
	// Stamina Attributes
	// ========================================
	
	/** Current stamina value (used for sprinting, dodging, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, Stamina);

	/** Maximum stamina value */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, MaxStamina);
	
	// ========================================
	// Ultimate Attributes
	// ========================================
	
	/** Current ultimate charge (builds up to enable ultimate ability) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Ultimate")
	FGameplayAttributeData UltimateCharge;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, UltimateCharge);
	
	/** Maximum ultimate charge required to use ultimate */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Ultimate")
	FGameplayAttributeData MaxUltimateCharge;
	ATTRIBUTE_ACCESSORS(UNeonAttributeSet, MaxUltimateCharge);
};