#pragma once
#include "NativeGameplayTags.h"

namespace CombatTags
{
	// Ability events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_LightAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_HeavyAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Block);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Roll);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Finisher);
	
	// State tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Exploration);
	// Combat state — for cancellation/gating once the combo system exists
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InCombat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Stunned);
	
	// Combat events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitboxOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitboxClose);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_BlockWindowOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_BlockWindowClose);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_ComboWindowOpen);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Block);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Evasion);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
}