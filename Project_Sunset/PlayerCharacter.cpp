#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"

/**
 * Constructor - Initializes all components and default values.
 */
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ========================================
	// Camera Setup
	// ========================================
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300; // Distance from character
	SpringArm->bUsePawnControlRotation = true; // Rotate with controller
	SpringArm->SocketOffset = FVector(0.f, 50.f, 50.f); // Offset camera position
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Let spring arm handle rotation

	// ========================================
	// Character Rotation Setup
	// ========================================
	bUseControllerRotationYaw = true; // Character rotates with camera
	GetCharacterMovement()->bOrientRotationToMovement = false; // Don't auto-rotate to movement
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // Rotation speed
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // Base walk speed

	// ========================================
	// Gameplay Ability System Setup
	// ========================================
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true); // Enable for multiplayer
	
	Attributes = CreateDefaultSubobject<UNeonAttributeSet>(TEXT("Attributes"));
}

/**
 * Called when the character spawns.
 * Sets up GAS bindings and attribute change notifications.
 */
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("=== PlayerCharacter::BeginPlay START for %s ==="), *GetName());

	if (AbilitySystemComponent)
	{
		// Initialize the Ability System Component
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (Attributes)
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: Attributes found, setting up bindings..."));
			
			// ========================================
			// Bind Attribute Change Delegates
			// ========================================
			// These fire whenever an attribute value changes through GAS
			
			// Health changes → OnHealthChangedNative → OnHealthChanged (Blueprint)
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				Attributes->GetHealthAttribute()
			).AddUObject(this, &APlayerCharacter::OnHealthChangedNative);
			
			// Neon changes → OnNeonChangedNative → OnNeonChanged (Blueprint)
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				Attributes->GetNeonAttribute()
			).AddUObject(this, &APlayerCharacter::OnNeonChangedNative);
			
			// Stamina changes → OnStaminaChangedNative → OnStaminaChanged (Blueprint)
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				Attributes->GetStaminaAttribute()
			).AddUObject(this, &APlayerCharacter::OnStaminaChangedNative);
			
			// Ultimate Charge changes → OnUltimateChargeChangedNative → OnUltimateChargeChanged (Blueprint)
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				Attributes->GetUltimateChargeAttribute()
			).AddUObject(this, &APlayerCharacter::OnUltimateChargeChangedNative);
			
			// ========================================
			// Bind Damage Delegate
			// ========================================
			// This fires when damage is dealt (from PostGameplayEffectExecute)
			Attributes->OnDamageTaken.AddDynamic(this, &APlayerCharacter::HandleDamageTaken);
			UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: BOUND to OnDamageTaken delegate!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: ERROR - Attributes is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerCharacter: ERROR - AbilitySystemComponent is NULL!"));
	}
	
	UE_LOG(LogTemp, Error, TEXT("=== PlayerCharacter::BeginPlay COMPLETE ==="));
}

/**
 * Applies the default Gameplay Effect to initialize attribute values.
 * This is where starting Health, Neon, Stamina, etc. get set.
 */
void APlayerCharacter::InitializeAttributes()
{
	// Validate we have both an ASC and a Gameplay Effect to apply
	if (!AbilitySystemComponent || !DefaultAttributeEffect)
	{
		return;
	}

	// Create effect context (who is applying this effect)
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	// Create the effect specification (the "instruction manual")
	FGameplayEffectSpecHandle SpecHandle = 
		AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.0f, ContextHandle);

	// Apply the effect to ourselves
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
			*SpecHandle.Data.Get(), 
			AbilitySystemComponent
		);
	}
}

/**
 * Native callback for Health changes.
 * Routes to Blueprint event with both current and max values.
 */
void APlayerCharacter::OnHealthChangedNative(const FOnAttributeChangeData& Data)
{
	float NewValue = Data.NewValue;
	float MaxValue = AbilitySystemComponent->GetNumericAttribute(Attributes->GetMaxHealthAttribute());
	OnHealthChanged(NewValue, MaxValue);
}

/**
 * Native callback for Neon changes.
 * Routes to Blueprint event with both current and max values.
 */
void APlayerCharacter::OnNeonChangedNative(const FOnAttributeChangeData& Data)
{
	float NewValue = Data.NewValue;
	float MaxValue = AbilitySystemComponent->GetNumericAttribute(Attributes->GetMaxNeonAttribute());
	OnNeonChanged(NewValue, MaxValue);
}

/**
 * Native callback for Stamina changes.
 * Routes to Blueprint event with both current and max values.
 */
void APlayerCharacter::OnStaminaChangedNative(const FOnAttributeChangeData& Data)
{
	float NewValue = Data.NewValue;
	float MaxValue = AbilitySystemComponent->GetNumericAttribute(Attributes->GetMaxStaminaAttribute());
	OnStaminaChanged(NewValue, MaxValue);
}

/**
 * Native callback for Ultimate Charge changes.
 * Routes to Blueprint event with both current and max values.
 */
void APlayerCharacter::OnUltimateChargeChangedNative(const FOnAttributeChangeData& Data)
{
	float NewValue = Data.NewValue;
	float MaxValue = AbilitySystemComponent->GetNumericAttribute(Attributes->GetMaxUltimateChargeAttribute());
	OnUltimateChargeChanged(NewValue, MaxValue);
}

/**
 * Base implementation of damage handler.
 * PlayerCharacter does nothing (player damage is handled in Blueprint).
 * EnemyCharacter overrides this for AI reactions.
 */
void APlayerCharacter::HandleDamageTaken(float DamageAmount, AActor* DamagedActor)
{
	UE_LOG(LogTemp, Error, TEXT("=== PlayerCharacter::HandleDamageTaken CALLED ==="));
	UE_LOG(LogTemp, Error, TEXT("DamageAmount: %.1f, DamagedActor: %s, This: %s"), 
		DamageAmount, 
		DamagedActor ? *DamagedActor->GetName() : TEXT("NULL"), 
		*GetName());
	
	// Base implementation - intentionally empty
	// Player damage reactions are handled in Blueprint
	// Enemy damage reactions override this function
}

/**
 * Called when a controller possesses this character.
 * Initializes GAS and sets up Enhanced Input.
 */
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize GAS for server/AI
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAttributes();
	}

	// Set up Enhanced Input System (player only)
	if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

/**
 * Sets up input bindings for Enhanced Input System.
 */
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind movement input
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(
				MoveAction, 
				ETriggerEvent::Triggered, 
				this, 
				&APlayerCharacter::Move
			);
		}
		
		// Bind camera look input
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(
				LookAction, 
				ETriggerEvent::Triggered, 
				this, 
				&APlayerCharacter::Look
			);
		}
		
		// Bind sprint input (press and release)
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction, 
				ETriggerEvent::Started, 
				this, 
				&APlayerCharacter::StartSprint
			);
			EnhancedInputComponent->BindAction(
				SprintAction, 
				ETriggerEvent::Completed, 
				this, 
				&APlayerCharacter::StopSprint
			);
		}
	}
}

/**
 * Handles movement input from WASD/Left Stick.
 * Applies movement relative to camera rotation.
 */
void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// Get camera rotation (where we're looking)
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// Get forward and right vectors relative to camera
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		
		// Apply movement input
		AddMovementInput(ForwardDirection, MovementVector.Y); // Forward/Backward
		AddMovementInput(RightDirection, MovementVector.X);   // Left/Right
	}
}

/**
 * Handles camera look input from Mouse/Right Stick.
 */
void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);   // Horizontal rotation
		AddControllerPitchInput(LookAxisVector.Y); // Vertical rotation
	}
}

/**
 * Increases movement speed for sprinting.
 */
void APlayerCharacter::StartSprint()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f; // Sprint speed
	}
}

/**
 * Returns movement speed to normal walking.
 */
void APlayerCharacter::StopSprint()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.f; // Walk speed
	}
}

/**
 * Returns the Ability System Component.
 * Required by IAbilitySystemInterface.
 */
UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

/**
 * Called every frame.
 */
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}