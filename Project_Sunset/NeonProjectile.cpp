#include "NeonProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"

/**
 * Constructor - Sets up all components and default values.
 */
ANeonProjectile::ANeonProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// ========================================
	// Collision Component Setup
	// ========================================
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// Ignore all channels by default
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// Enable overlap with characters/enemies
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// Block against world geometry (causes OnHit event)
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	RootComponent = CollisionComponent;

	// ========================================
	// Mesh Component Setup
	// ========================================
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Visual only

	// ========================================
	// Projectile Movement Setup
	// ========================================
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; // Face direction of travel
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity
	
	// Auto-destroy after 10 seconds to prevent infinite projectiles
	InitialLifeSpan = 10.0f;
}

/**
 * Called when the actor is spawned.
 * Binds collision event handlers.
 */
void ANeonProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind collision callbacks
	CollisionComponent->OnComponentHit.AddDynamic(this, &ANeonProjectile::OnProjectileHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANeonProjectile::OnProjectileOverlap);
}

/**
 * Called every frame - handles boomerang return logic.
 */
void ANeonProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only process boomerang logic if we're in boomerang mode
	if (!bIsBoomerang || !BoomerangOwner)
	{
		return;
	}

	// ========================================
	// Outgoing Phase Logic
	// ========================================
	if (BoomerangPhase == EProjectilePhase::Outgoing)
	{
		// Check if we've traveled far enough to start returning
		float DistanceTraveled = FVector::Dist(BoomerangStartLocation, GetActorLocation());
		
		if (DistanceTraveled >= MaxTravelDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("=== BOOMERANG ENTERING RETURN PHASE (Distance) ==="));
			
			// Switch to return phase
			BoomerangPhase = EProjectilePhase::Returning;
			
			// Clear hit list so enemies can be hit again on return
			HitActorsThisPhase.Empty();
			
			// Enable homing to return to owner
			if (ProjectileMovement)
			{
				ProjectileMovement->bIsHomingProjectile = true;
				ProjectileMovement->HomingAccelerationMagnitude = 8000.f; // Strong homing to prevent orbit
				ProjectileMovement->HomingTargetComponent = BoomerangOwner->GetRootComponent();
			}
		}
	}
	// ========================================
	// Returning Phase Logic
	// ========================================
	else if (BoomerangPhase == EProjectilePhase::Returning)
	{
		// Check if we're close enough to owner to destroy
		float DistanceToOwner = FVector::Dist(GetActorLocation(), BoomerangOwner->GetActorLocation());
		
		if (DistanceToOwner < 100.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Boomerang returned to owner - destroying"));
			Destroy();
		}
	}
}

/**
 * Configures this projectile to behave as a boomerang.
 * Call this immediately after spawning the projectile.
 */
void ANeonProjectile::InitializeBoomerang(AActor* InOwner, float InMaxDistance)
{
	UE_LOG(LogTemp, Warning, TEXT("=== InitializeBoomerang called ==="));
	
	// Set boomerang parameters
	bIsBoomerang = true;
	BoomerangOwner = InOwner;
	MaxTravelDistance = InMaxDistance;
	BoomerangStartLocation = GetActorLocation();
	BoomerangPhase = EProjectilePhase::Outgoing;
	HitActorsThisPhase.Empty();
	
	// Configure for straight outgoing flight
	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = false; // Straight flight initially
		ProjectileMovement->HomingAccelerationMagnitude = 0.f;
		ProjectileMovement->InitialSpeed = 2000.f;
		ProjectileMovement->MaxSpeed = 2000.f;
		ProjectileMovement->ProjectileGravityScale = 0.f;
		
		UE_LOG(LogTemp, Warning, TEXT("Boomerang configured: MaxDistance=%.0f, Speed=%.0f"), 
			InMaxDistance, ProjectileMovement->InitialSpeed);
	}
}

/**
 * Handles blocking hits (walls, obstacles).
 * Standard projectiles destroy immediately.
 * Boomerangs enter return phase if they hit a wall during outgoing flight.
 */
void ANeonProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	// ========================================
	// Boomerang Wall Hit Logic
	// ========================================
	if (bIsBoomerang && BoomerangPhase == EProjectilePhase::Outgoing && Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boomerang hit wall: %s. Forcing Return."), 
			*OtherActor->GetName());

		// Switch to return phase early
		BoomerangPhase = EProjectilePhase::Returning;
		HitActorsThisPhase.Empty();

		// Enable homing back to owner
		if (ProjectileMovement && BoomerangOwner)
		{
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingAccelerationMagnitude = 8000.f;
			ProjectileMovement->HomingTargetComponent = BoomerangOwner->GetRootComponent();
		}
	}
	// ========================================
	// Standard Projectile Wall Hit Logic
	// ========================================
	else if (!bIsBoomerang)
	{
		// Standard projectiles destroy on any blocking hit
		Destroy();
	}
}

/**
 * Handles overlap events with characters/enemies.
 * Routes to shared collision logic.
 */
void ANeonProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// All overlap collision logic is handled in HandleCollisionLogic
	HandleCollisionLogic(OtherActor);
}

/**
 * Applies a Gameplay Effect to a target actor.
 * Handles all the GAS boilerplate for effect application.
 */
void ANeonProjectile::ApplyGameplayEffectToTarget(
	AActor* TargetActor, 
	TSubclassOf<UGameplayEffect> EffectClass)
{
	// Validate inputs
	if (!TargetActor || !EffectClass)
	{
		return;
	}

	// Get target's Ability System Component
	UAbilitySystemComponent* TargetASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	// Get source ASC (the player who fired this)
	UAbilitySystemComponent* SourceASC = nullptr;
	if (GetOwner())
	{
		SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	// Apply the effect if target has an ASC
	if (TargetASC)
	{
		// Create effect context
		FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		// Add instigator if we have a source
		if (SourceASC)
		{
			EffectContext.AddInstigator(GetOwner(), GetOwner());
		}

		// Create and apply the effect spec
		FGameplayEffectSpecHandle SpecHandle = 
			TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
		
		if (SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

/**
 * Main collision logic for both standard and boomerang projectiles.
 * Determines which effects to apply based on projectile type and phase.
 */
void ANeonProjectile::HandleCollisionLogic(AActor* OtherActor)
{
	// Ignore invalid targets and self-hits
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}
	
	// ========================================
	// Standard Projectile Behavior
	// ========================================
	if (!bIsBoomerang)
	{
		// Apply damage effect
		if (DamageEffectClass)
		{
			ApplyGameplayEffectToTarget(OtherActor, DamageEffectClass);
		}
		
		// Destroy projectile
		Destroy();
		return;
	}

	// ========================================
	// Boomerang Behavior
	// ========================================

	// Prevent hitting the same actor twice in one phase
	if (HitActorsThisPhase.Contains(OtherActor))
	{
		return;
	}

	// Only process actors with Ability System Components
	UAbilitySystemComponent* TargetASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC)
	{
		return;
	}

	// Track that we've hit this actor in this phase
	HitActorsThisPhase.Add(OtherActor);

	// Apply appropriate effect based on current phase
	if (BoomerangPhase == EProjectilePhase::Outgoing)
	{
		// Outgoing: Apply Corruption debuff
		if (CorruptionEffectClass)
		{
			ApplyGameplayEffectToTarget(OtherActor, CorruptionEffectClass);
			UE_LOG(LogTemp, Warning, 
				TEXT("Boomerang OUTGOING hit: %s - Applied Corruption!"), 
				*OtherActor->GetName());
		}
	}
	else if (BoomerangPhase == EProjectilePhase::Returning)
	{
		// Returning: Apply Damage (combo with corruption!)
		if (DamageEffectClass)
		{
			ApplyGameplayEffectToTarget(OtherActor, DamageEffectClass);
			UE_LOG(LogTemp, Warning, 
				TEXT("Boomerang RETURNING hit: %s - Applied Damage!"), 
				*OtherActor->GetName());
		}
	}
}