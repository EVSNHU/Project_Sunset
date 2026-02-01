#include "BaseTelegraphAbility.h"
#include "GameFramework/Character.h"

/**
 * Constructor - Required by Unreal's reflection system even if empty
 */
UBaseTelegraphAbility::UBaseTelegraphAbility() 
{
}

/**
 * Spawns a telegraph actor at the owner's location with forward offset.
 * The telegraph is attached to the owner and will move with them.
 */
void UBaseTelegraphAbility::StartTelegraph(TSubclassOf<AActor> TelegraphClassOverride)
{
	// Early exit if a telegraph is already active to prevent duplicates
	if (ActiveTelegraph)
	{
		return;
	}

	// Get the character/pawn that owns this ability
	AActor* Avatar = GetAvatarActorFromActorInfo();
	
	// Determine which class to spawn (override takes priority)
	TSubclassOf<AActor> ClassToSpawn = TelegraphClassOverride ? TelegraphClassOverride : DefaultTelegraphClass;

	// Validate we have both an owner and a class to spawn
	if (Avatar && ClassToSpawn)
	{
		// Calculate spawn position: owner's location, slightly below ground, offset forward
		FVector SpawnLoc = Avatar->GetActorLocation();
		SpawnLoc.Z -= 80.0f; // Lower to ground level (adjust as needed for your game)
		SpawnLoc += Avatar->GetActorForwardVector() * TelegraphForwardOffset;

		// Use owner's rotation for the telegraph
		FRotator SpawnRot = Avatar->GetActorRotation();

		// Configure spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Avatar;

		// Spawn the telegraph actor
		ActiveTelegraph = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLoc, SpawnRot, SpawnParams);
        
		if (ActiveTelegraph)
		{
			// Attach to owner so it follows their movement
			ActiveTelegraph->AttachToComponent(
				Avatar->GetRootComponent(), 
				FAttachmentTransformRules::KeepWorldTransform
			);
			
			// Apply configured scale
			ActiveTelegraph->SetActorScale3D(TelegraphScale);
		}
	}
}

/**
 * Destroys the active telegraph actor and clears the reference.
 */
void UBaseTelegraphAbility::StopTelegraph()
{
	if (ActiveTelegraph)
	{
		ActiveTelegraph->Destroy();
		ActiveTelegraph = nullptr;
	}
}