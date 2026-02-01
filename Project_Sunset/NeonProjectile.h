#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NeonProjectile.generated.h"

// Forward declarations to avoid circular dependencies
class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UGameplayEffect;

/**
 * Enum defining the two phases of a boomerang projectile's flight path.
 */
UENUM(BlueprintType)
enum class EProjectilePhase : uint8
{
	/** Projectile is traveling away from the owner */
	Outgoing UMETA(DisplayName = "Outgoing"),
	
	/** Projectile is returning back to the owner */
	Returning UMETA(DisplayName = "Returning")
};

/**
 * Projectile actor that can function as either a standard projectile or a boomerang.
 * 
 * Standard Mode:
 * - Flies in a straight line
 * - Applies damage on hit
 * - Destroys on collision
 * 
 * Boomerang Mode:
 * - Flies out to max distance, then returns to owner
 * - Applies Corruption effect on outgoing flight
 * - Applies Damage effect on return flight
 * - Can hit each enemy once per phase (twice total)
 */
UCLASS()
class PROJECT_SUNSET_API ANeonProjectile : public AActor
{
	GENERATED_BODY()
	
public:    
	ANeonProjectile();

	/** Called every frame to update boomerang return logic */
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// Components
	// ========================================
	
	/** Sphere collision component for detecting hits */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	/** Handles projectile movement physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** Visual mesh for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// ========================================
	// Gameplay Effects
	// ========================================
	
	/** Gameplay Effect applied on hit (standard) or on return (boomerang) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Gameplay Effect applied on outgoing flight (boomerang only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> CorruptionEffectClass;

	// ========================================
	// Boomerang Properties
	// ========================================
	
	/** Whether this projectile uses boomerang behavior */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang", Meta = (ExposeOnSpawn = true))
	bool bIsBoomerang = false;

	/** The actor that spawned this boomerang (used for return target) */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang")
	AActor* BoomerangOwner = nullptr;

	/** Maximum distance the boomerang travels before returning */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang")
	float MaxTravelDistance = 1000.f;

	/** Location where the boomerang was spawned (used to calculate distance) */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang")
	FVector BoomerangStartLocation;

	/** Current phase of boomerang flight (Outgoing or Returning) */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang")
	EProjectilePhase BoomerangPhase = EProjectilePhase::Outgoing;

	/** Tracks actors hit during current phase (prevents double-hitting same target) */
	UPROPERTY(BlueprintReadWrite, Category = "Boomerang")
	TSet<AActor*> HitActorsThisPhase;

	/**
	 * Initializes this projectile as a boomerang.
	 * Must be called after spawning if you want boomerang behavior.
	 * 
	 * @param InOwner - The actor that spawned this projectile (return target)
	 * @param InMaxDistance - How far the projectile travels before returning
	 */
	UFUNCTION(BlueprintCallable, Category = "Boomerang")
	void InitializeBoomerang(AActor* InOwner, float InMaxDistance);

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Handles blocking collisions (walls, obstacles).
	 * Standard projectiles destroy on hit.
	 * Boomerangs switch to return phase if they hit a wall during outgoing phase.
	 */
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
	);

	/**
	 * Handles overlap collisions (characters, enemies).
	 * Routes to HandleCollisionLogic for processing.
	 */
	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
	);

private:
	/**
	 * Applies a Gameplay Effect to a target actor.
	 * Handles ASC retrieval and effect context setup.
	 * 
	 * @param TargetActor - Actor to apply the effect to
	 * @param EffectClass - The Gameplay Effect class to apply
	 */
	void ApplyGameplayEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass);

	/**
	 * Main collision logic handler for both standard and boomerang projectiles.
	 * Determines which effects to apply based on projectile mode and phase.
	 * 
	 * @param OtherActor - The actor that was hit
	 */
	void HandleCollisionLogic(AActor* OtherActor);
};