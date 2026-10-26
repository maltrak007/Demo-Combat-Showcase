# Demo Combat Showcase

🚧 **Work in progress** — a Gameplay Ability System combat framework in Unreal Engine (C++), built as a portfolio piece to show how I design, build and document combat.

> Combat animations come from licensed Fab packs and are excluded per their terms. See the demo video for full functionality.

## What it demonstrates

- **GAS-based melee combat** — abilities for block, counter-attack, evade, finisher and hit reaction on a shared melee base (`GA_MeleeAbilityBase`).
- **Animation-driven windows** — notify states for combo windows, hitboxes, gameplay-event windows and granting gameplay tags.
- **Multi-bone hit tracing** — `AbilityTask_MultiBoneHitTrace` for accurate weapon and limb hits.
- **Components** — combat stats and combat feedback (game feel) components, plus an attribute set and central gameplay tags.
- **Enemy AI** — enemy base character, AI controller and data-driven behaviour/combat data.
- **HUD** — stat widgets bound to attributes.

## Design case studies

`Source/GLA_CombatSystem/CombatShowcase/DesignCase/` documents the *why* behind each mechanic. Case 01 ("From a Fake Block to Two Real Playstyles") is written; cases 02–05 are in progress.

## Structure

```
Source/GLA_CombatSystem/CombatShowcase/
  Core/Combat/   Characters (player, enemy), animations, components, data, interfaces
  Core/GAS/      Abilities, ability tasks, attribute set, tags
  Core/HUD/      Stat widgets
  DesignCase/    Design write-ups
```

## Author

**Gonzalo Liñán Aldana** — [GitHub](https://github.com/maltrak007)
