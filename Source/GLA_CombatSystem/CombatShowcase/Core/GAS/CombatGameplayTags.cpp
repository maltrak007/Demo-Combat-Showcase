#include "CombatGameplayTags.h"

namespace CombatTags
{
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_LightAttack, "Event.Ability.LightAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_HeavyAttack, "Event.Ability.HeavyAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Block, "Event.Ability.Block");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Roll, "Event.Ability.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Finisher, "Event.Ability.Finisher");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Exploration, "State.Exploration");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_InCombat, "State.Combat.InCombat");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Stunned, "State.Combat.Stunned");
	

	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitboxOpen, "Event.Combat.HitboxOpen");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitboxClose, "Event.Combat.HitboxClose");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact, "Event.Combat.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
}