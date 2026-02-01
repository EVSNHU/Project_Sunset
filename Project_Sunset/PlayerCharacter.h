#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h" 
#include "AbilitySystemComponent.h"
#include "InputActionValue.h"
#include "NeonAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "PlayerCharacter.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class UAbilitySystemComponent;
class UNeonAttributeSet;
class UInputMappingContext;
class UInputAction;

/**
 * Base character class implementing the Gameplay Ability System (GAS).
 * Handles:
 * - Camera setup (third-person)
 * - Enhanced Input System
 * - GAS integration (abilities, attributes, effects)
 * - Movement (walk, sprint)
 * - Attribute change notifications (Health, Neon, Stamina, Ultimate)
 * 
 * Both player and enemy characters inherit from this class.
 */
UCLASS()
class PROJECT_SUNSET_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	
	// ========================================
	// Camera Components
	// ========================================
	
	/** Spring arm for smooth camera following */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	/** Third-person camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	
	// ========================================
	// Gameplay Ability System Components
	// ========================================
	
	/** Core GAS component - manages abilities and effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute set containing Health, Neon, Stamina, etc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UNeonAttributeSet* Attributes;
	
	// ========================================
	// Blueprint Events (Attribute Changes)
	// ========================================
	
	/**
	 * Blueprint event called when Health changes.
	 * Use this to update UI health bars.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnHealthChanged(float NewHealth, float MaxHealth);

	/**
	 * Blueprint event called when Neon changes.
	 * Use this to update UI mana/style meters.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnNeonChanged(float NewNeon, float MaxNeon);

	/**
	 * Blueprint event called when Stamina changes.
	 * Use this to update UI stamina bars.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnStaminaChanged(float NewStamina, float MaxStamina);

	/**
	 * Blueprint event called when Ultimate Charge changes.
	 * Use this to update UI ultimate meters.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnUltimateChargeChanged(float NewUltimate, float MaxUltimate);

	// ========================================
	// Movement Functions
	// ========================================
	
	/** Increases movement speed to sprint speed (600) */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/** Decreases movement speed to walk speed (300) */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	// ========================================
	// Enhanced Input Assets
	// ========================================
	
	/** Input mapping context (contains all input bindings) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	/** Movement input action (WASD / Left Stick) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Camera look input action (Mouse / Right Stick) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	/** Sprint input action (Shift / Button) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	/** 
	 * Returns the Ability System Component.
	 * Required by IAbilitySystemInterface.
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;
	
	/** Called when this character is possessed by a controller */
	virtual void PossessedBy(AController* NewController) override;
	
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
	
	/** Sets up input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========================================
	// Input Handlers
	// ========================================
	
	/** Handles movement input (WASD / Left Stick) */
	void Move(const FInputActionValue& Value);
	
	/** Handles camera look input (Mouse / Right Stick) */
	void Look(const FInputActionValue& Value);

	// ========================================
	// GAS Initialization
	// ========================================
	
	/** 
	 * Gameplay Effect that sets initial attribute values.
	 * Assign in Blueprint (e.g., GE_InitializeStats).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DefaultAttributeEffect;

	/** Applies the DefaultAttributeEffect to initialize stats */
	void InitializeAttributes();

	// ========================================
	// Attribute Change Callbacks
	// ========================================
	
	/** Native C++ callback for Health changes - routes to Blueprint event */
	virtual void OnHealthChangedNative(const FOnAttributeChangeData& Data);
	
	/** Native C++ callback for Neon changes - routes to Blueprint event */
	virtual void OnNeonChangedNative(const FOnAttributeChangeData& Data);
	
	/** Native C++ callback for Stamina changes - routes to Blueprint event */
	virtual void OnStaminaChangedNative(const FOnAttributeChangeData& Data);
	
	/** Native C++ callback for Ultimate Charge changes - routes to Blueprint event */
	virtual void OnUltimateChargeChangedNative(const FOnAttributeChangeData& Data);
	
	/**
	 * Called when this character takes damage.
	 * Virtual so EnemyCharacter can override for custom behavior.
	 * 
	 * @param DamageAmount - Amount of damage received
	 * @param DamagedActor - The actor that was damaged
	 */
	UFUNCTION()
	virtual void HandleDamageTaken(float DamageAmount, AActor* DamagedActor);
};