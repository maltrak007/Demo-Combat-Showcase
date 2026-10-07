#include "CombatGameplayTags.h"

namespace CombatTags
{
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_LightAttack, "Event.Ability.LightAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_HeavyAttack, "Event.Ability.HeavyAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Counter, "Event.Ability.Counter");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Block, "Event.Ability.Block");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_BackStep, "Event.Ability.BackStep");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Roll, "Event.Ability.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Finisher, "Event.Ability.Finisher");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Exploration, "State.Exploration");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_InCombat, "State.Combat.InCombat");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Stunned, "State.Combat.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Invincible, "State.Combat.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Telegraphing, "State.Combat.Telegraphing");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_MovementLocked, "State.Combat.MovementLocked");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Executable, "State.Combat.Executable");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_BeingExecuted, "State.Combat.BeingExecuted");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Blocking_DirectionUp, "State.Blocking.DirectionUp");
	UE_DEFINE_GAMEPLAY_TAG(State_Blocking_DirectionDown, "State.Blocking.DirectionDown");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_ParryWindowOpen, "State.Combat.ParryWindowOpen");
	UE_DEFINE_GAMEPLAY_TAG(State_Stamina_RegenBlocked, "State.Stamina.RegenBlocked");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitboxOpen, "Event.Combat.HitboxOpen");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitboxClose, "Event.Combat.HitboxClose");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact, "Event.Combat.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_ComboWindowOpen, "Event.Combat.ComboWindowOpen");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_BlockDirectionChanged, "Event.Combat.BlockDirectionChanged");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact_Unblocked, "Event.Combat.HitReact.Unblocked");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact_Blocked, "Event.Combat.HitReact.Blocked");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact_Parried, "Event.Combat.HitReact.Parried");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact_ParrySuccess, "Event.Combat.HitReact.ParrySuccess");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact_GuardBreak, "Event.Combat.HitReact.GuardBreak");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_ReactionFinished, "Event.Combat.ReactionFinished");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Attack, "Ability.Type.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Block, "Ability.Type.Block");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Evasion, "Ability.Type.Evasion");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Finisher, "Ability.Type.Finisher");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_HitReaction, "Ability.Type.HitReaction");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_StaminaConsumed, "Data.StaminaConsumed");
}