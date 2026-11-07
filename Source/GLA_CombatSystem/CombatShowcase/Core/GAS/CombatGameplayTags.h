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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_EquipWeapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Aim);

	// Misc Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Exploration);
	// Combat state — for cancellation/gating once the combo system exists
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InCombat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Telegraphing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_MovementLocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Executable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_BeingExecuted);
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_WeaponCommit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Fire);

	// Ability Type Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Block);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Evasion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Finisher);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_HitReaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Equip);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Aim);

	// Data tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_StaminaConsumed);

	// Enemy Type (This Defines AI Behavior)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Type_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Type_Ranged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Type_Mixed);

	// Archetype Enemy Type (This Defines Moveset)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Archetype_Brawler);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Archetype_Kicker);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Archetype_Defensive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Archetype_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Archetype_Weapon);

	// Enemy Mastery (This Defines AI Difficulty)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Mastery_Novice);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Mastery_Intermediate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Mastery_Advanced);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Mastery_Master);

	// Weapon Tag
	// Parents of the weapon tags, so a weapon can be tested with MatchesTag (the director uses Weapon_Type_Ranged).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Ranged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Unarmed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Knife);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Spear);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Axe);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Axe_SingleAxe);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Axe_DualAxe);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Sword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Sword_SingleSword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Sword_SwordAndShield);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Sword_HeavySword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Melee_Sword_Katana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Ranged_Pistol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Ranged_Rifle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Ranged_Shotgun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Type_Ranged_Sniper);
}
