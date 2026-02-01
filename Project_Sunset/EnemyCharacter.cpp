#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NeonAttributeSet.h"
#include "AbilitySystemComponent.h"

/**
 * Constructor - Sets up basic enemy movement speed
 */
AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set enemy movement speed (slower than player's 300 base, 600 sprint)
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

/**
 * Initializes the enemy when spawned.
 * Calls parent BeginPlay (which sets up GAS and binds damage delegate),
 * then initializes enemy-specific attribute values.
 */
void AEnemyCharacter::BeginPlay()
{
	// IMPORTANT: Call parent BeginPlay first
	// This sets up AbilitySystemComponent and binds the damage delegate
	Super::BeginPlay();

	// Initialize enemy health values
	if (Attributes)
	{
		Attributes->InitHealth(100.0f);
		Attributes->InitMaxHealth(100.0f);
		Attributes->InitNeon(0.0f); // Enemies start with no Neon
	}
    
	UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter BeginPlay complete"));
}

/**
 * Handles damage received by this enemy.
 * Filters out damage events meant for other actors, then triggers Blueprint event.
 * 
 * @param DamageAmount - Amount of damage received
 * @param DamagedActor - The actor that was damaged
 */
void AEnemyCharacter::HandleDamageTaken(float DamageAmount, AActor* DamagedActor)
{
	// Safety check: Only process damage if it's meant for this specific enemy
	if (DamagedActor != this)
	{
		return;
	}
    
	UE_LOG(LogTemp, Error, TEXT("EnemyCharacter: %s took %.1f damage!"), *GetName(), DamageAmount);
    
	// Trigger Blueprint event for AI/animation reactions
	DamageEvent(DamageAmount);
}