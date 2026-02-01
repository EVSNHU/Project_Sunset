#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * Enemy character class that inherits from PlayerCharacter.
 * Shares the same GAS (Gameplay Ability System) setup but with different damage handling.
 * Enemies can receive damage and trigger Blueprint events for AI reactions.
 */
UCLASS()
class PROJECT_SUNSET_API AEnemyCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Blueprint event that fires when this enemy takes damage.
	 * Implement in Blueprint to trigger animations, AI reactions, effects, etc.
	 * 
	 * @param DamageAmount - The amount of damage received
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void DamageEvent(float DamageAmount);

	/**
	 * Overridden damage handler from parent class.
	 * Filters damage to only affect this specific enemy, then triggers Blueprint event.
	 * 
	 * @param DamageAmount - The amount of damage received
	 * @param DamagedActor - The actor that was damaged (should be this enemy)
	 */
	virtual void HandleDamageTaken(float DamageAmount, AActor* DamagedActor) override;
};