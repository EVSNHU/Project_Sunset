# Project Sunset

Action combat system built with Unreal Engine 5's Gameplay Ability System (GAS), demonstrating advanced gameplay programming with C++ and Blueprint integration.

![Gameplay Demo](link-to-video-or-gif)

## Overview

Project Sunset is a portfolio piece showcasing systemic combat design, resource management, and ability framework implementation using UE5's Gameplay Ability System.

## Key Features

### Combat Systems
- **Telegraph-Based Targeting**: Hold-to-aim system with visual indicators
- **Boomerang Projectile**: Dual-phase collision (corruption on outward, damage on return)
- **Systemic Damage**: Corruption status + Neon damage = 2.5x amplification
- **Multi-Hit Combos**: Basic attacks (4-hit) and heavy slash (4-hit with neon damage)
- **Ground-Targeted Ultimate**: AOE slam with aerial movement and targeting decal

### Resource Management
- **Stamina System**: Governs attacks, dodging, sliding, and double jumping
- **Context-Sensitive Regen**: Instant regeneration vs 2-second exhaustion penalty
- **Charge-Based Dodge**: 2 independent charges with stacking cooldowns

### Technical Implementation
- Custom C++ `AttributeSet` for health, stamina, and resource attributes
- Custom `GameplayEffectExecutionCalculation` for systemic damage interactions
- Parent ability class (`GA_BaseTelegraphAbility`) for reusable targeting patterns
- Tag-based ability gating and requirements
- Gameplay Effect stacking for charge management

## Technical Highlights

### Custom C++ Classes

**NeonAttributeSet.cpp/h**
- Manages health, stamina, neon, and ultimate charge attributes
- Implements attribute clamping in `PostGameplayEffectExecute`
- Broadcasts damage events via delegates

**NeonDamageExecCalculation.cpp/h**
- Custom damage calculation with modifier support
- Checks for Corruption status + Neon damage combo
- Applies 2.5x damage amplification systemically

**NeonProjectile.cpp/h**
- Dual-phase boomerang behavior with independent collision handling
- Separates blocking hits from overlap events
- Phase-based gameplay effect application

**PlayerCharacter.cpp/h & EnemyCharacter.cpp/h**
- GAS integration with `IAbilitySystemInterface`
- Attribute change delegates for UI updates
- Damage event handling

**BaseTelegraphAbility.cpp/h**
- Reusable parent class for hold-to-aim abilities
- Manages telegraph spawning and cleanup

### Architecture Decisions

**Why GAS?**
- Data-driven ability design
- Scalable for team environments
- Industry-standard framework

**Systemic Design Philosophy**
- Abilities interact meaningfully (corruption enables amplification)
- Resources gate player choices (stamina prevents spam)
- Charges enable aggressive play without breaking balance

## Development Timeline

**Week 1:** Core combat (basic attacks, heavy slash, projectile)  
**Week 2:** GAS integration (attributes, effects, abilities)  
**Week 3:** Resource systems (stamina, charges, cooldowns)  
**Week 4:** Ultimate ability and polish

## Tech Stack

- **Engine:** Unreal Engine 5.6
- **Languages:** C++, Blueprint
- **Framework:** Gameplay Ability System (GAS)
- **Tools:** Visual Studio 2022, Rider

## What I Learned

- GAS cooldown system doesn't support charge-based abilities natively - built custom solution using effect stacking
- Attribute clamping in `PostGameplayEffectExecute` is critical for resource management
- Separation of concerns between C++ (collision, attributes) and Blueprint (gameplay logic) enables rapid iteration
- Tag-based ability requirements provide clean, data-driven gating

## Future Improvements

- Ultimate charge system with collectible orbs
- Enemy AI behaviors
- Dodge invincibility frames
- Animation polish and blending

## Contact

Enrique - [LinkedIn](your-linkedin) | [Portfolio](your-site)

## Note

This repository contains C++ source code only. Full project includes Blueprint assets, materials, and animations not included in this repo due to size constraints.
