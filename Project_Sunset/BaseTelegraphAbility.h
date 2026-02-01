#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BaseTelegraphAbility.generated.h"

/**
 * Base class for abilities that display a telegraph (visual indicator) before executing.
 * This class manages spawning and destroying telegraph actors during ability execution.
 * Designed to be used with Blueprint child classes that control the actual ability flow.
 */
UCLASS()
class PROJECT_SUNSET_API UBaseTelegraphAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBaseTelegraphAbility();

protected:
	// ========================================
	// Blueprint-Callable Functions
	// ========================================
	
	/**
	 * Spawns and attaches a telegraph actor to the ability owner.
	 * Call this at the start of your ability animation/montage.
	 * 
	 * @param TelegraphClassOverride - Optional class to spawn instead of DefaultTelegraphClass
	 */
	UFUNCTION(BlueprintCallable, Category = "Telegraph")
	void StartTelegraph(TSubclassOf<AActor> TelegraphClassOverride = nullptr);

	/**
	 * Destroys the active telegraph actor.
	 * Call this when the ability hits, completes, or is cancelled.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telegraph")
	void StopTelegraph();

	// ========================================
	// Configuration Properties
	// ========================================
	
	/** Default telegraph actor class to spawn (e.g., a decal or mesh) */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	TSubclassOf<AActor> DefaultTelegraphClass;

	/** Scale to apply to the spawned telegraph actor */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FVector TelegraphScale = FVector(1.0f, 1.0f, 1.0f);

	/** Distance to offset the telegraph forward from the owner's position */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	float TelegraphForwardOffset = 100.0f;

private:
	/** Reference to the currently active telegraph actor, if any */
	UPROPERTY()
	AActor* ActiveTelegraph;
};