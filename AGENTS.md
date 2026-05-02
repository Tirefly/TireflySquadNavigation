# TireflySquadNavigation Plugin Instructions

This plugin owns the default OpenSpec workspace for the `SquadNavDev` repository.

- Unless the user explicitly says otherwise, all OpenSpec proposals, specs, changes, tasks, design docs, validation, and archive work in this repo target `Plugins/TireflySquadNavigation`.
- Run OpenSpec CLI from `Plugins/TireflySquadNavigation` as the working root.
- The plugin-local OpenSpec content lives at `Plugins/TireflySquadNavigation/openspec/`.
- Unless a document says otherwise, treat paths inside this plugin as relative to `Plugins/TireflySquadNavigation`.

<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this plugin.

Always open `@/Plugins/TireflySquadNavigation/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/Plugins/TireflySquadNavigation/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions for the TSN plugin OpenSpec workspace.

<!-- OPENSPEC:END -->

## Authoritative Docs

- `Documents/战场小队导航系统 - 技术方案文档 V2.md` is the authoritative architecture document.
- `Documents/StateTree 测试执行指南.md` is the minimal StateTree setup and troubleshooting guide.
- `Documents/测试模块说明.md` explains the Editor-only test module, demo assets, and automation coverage.
- `README.md` is the quick-start overview; when it conflicts with the technical design doc, follow the design doc.

## Architecture And Naming

- The plugin is organized into four layers: decision, engagement slot, navigation, and movement.
- Use `TSN` as the plugin abbreviation and `Tsn` as the C++ prefix.
- Keep the semantic prefixes `Stance`, `Engagement`, and `Tactical` distinct; do not mix them.
- Host-project integration must go through `ITsnTacticalUnit`; do not cast plugin code to host-specific gameplay classes.

## Module Boundaries

- `Source/TireflySquadNavigation` is the runtime module. Reusable components, subsystems, NavArea classes, BehaviorTree nodes, and runtime StateTree nodes belong here.
- `Source/TireflySquadNavigationTest` is `EditorNoCommandlet` only. Demo actors, automation tests, and test-only BT/StateTree nodes belong here.
- Test-only StateTree nodes such as target selection or simulated attack must stay in the test module, not the runtime API surface.
- Demo content lives under `Content/FunctionShowcase/`; prefer these assets for behavior checks before editing host-project gameplay.

## Development Workflow

- After adding or removing C++ files, refresh project files before compiling.
- Preferred compile validation is `SquadNavDevEditor Win64 Development`.
- Prefer the `FunctionShowcase` maps for behavior smoke tests, then use automation tests for reusable logic.
- For StateTree AI, the `StateTree AI Component` belongs on the AIController, and AI logic must not auto-start before `Possess` provides the required context.