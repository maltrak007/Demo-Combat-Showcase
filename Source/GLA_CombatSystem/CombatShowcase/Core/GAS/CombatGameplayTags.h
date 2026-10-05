#pragma once
#include "NativeGameplayTags.h"

namespace CombatTags
{
	// Ability events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_LightAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_HeavyAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Counter);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Block);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_BackStep);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Roll);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Finisher);
	
	// Misc Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Exploration);
	// Combat state — for cancellation/gating once the combo system exists
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InCombat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Telegraphing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_MovementLocked);
	// Block / Parry Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Blocking_DirectionUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Blocking_DirectionDown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_ParryWindowOpen);
	// Stamina Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stamina_RegenBlocked);
	
	// Combat events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitboxOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitboxClose);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_ComboWindowOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_BlockDirectionChanged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact_Unblocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact_Blocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact_Parried);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact_ParrySuccess);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact_GuardBreak);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_ReactionFinished);
	// Ability Type Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Block);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Evasion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Finisher);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_HitReaction);
	
	// Data tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_StaminaConsumed);
}