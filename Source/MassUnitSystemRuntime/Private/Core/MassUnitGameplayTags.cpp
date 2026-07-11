// Copyright Digi Logic Labs LLC. All Rights Reserved.

#include "Core/MassUnitGameplayTags.h"

#include "NativeGameplayTags.h"

namespace UE::MassUnitSystem::Tags
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UnitTypeDefault, "MassUnit.Type.Default");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UnitClassSoldier, "MassUnit.Class.Soldier");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_BehaviorIdle, "MassUnit.Behavior.Idle");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_FormationLine, "MassUnit.Formation.Line");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_FactionNeutral, "MassUnit.Faction.Neutral");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationIdle, "MassUnit.Animation.Idle");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationWalk, "MassUnit.Animation.Walk");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationRun, "MassUnit.Animation.Run");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationAttack, "MassUnit.Animation.Attack");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationDeath, "MassUnit.Animation.Death");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationStun, "MassUnit.Animation.Stun");

	FGameplayTag UnitTypeDefault() { return TAG_UnitTypeDefault.GetTag(); }
	FGameplayTag UnitClassSoldier() { return TAG_UnitClassSoldier.GetTag(); }
	FGameplayTag BehaviorIdle() { return TAG_BehaviorIdle.GetTag(); }
	FGameplayTag FormationLine() { return TAG_FormationLine.GetTag(); }
	FGameplayTag FactionNeutral() { return TAG_FactionNeutral.GetTag(); }
	FGameplayTag AnimationIdle() { return TAG_AnimationIdle.GetTag(); }
	FGameplayTag AnimationWalk() { return TAG_AnimationWalk.GetTag(); }
	FGameplayTag AnimationRun() { return TAG_AnimationRun.GetTag(); }
	FGameplayTag AnimationAttack() { return TAG_AnimationAttack.GetTag(); }
	FGameplayTag AnimationDeath() { return TAG_AnimationDeath.GetTag(); }
	FGameplayTag AnimationStun() { return TAG_AnimationStun.GetTag(); }
}
