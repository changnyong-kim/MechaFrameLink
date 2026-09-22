# MechaFrameLink Agent Guidelines

## Project

- Project name: `MechaFrameLink`
- Engine: Unreal Engine 5
- This project is both:
  - an Unreal Engine portfolio project
  - an AI-assisted development workflow project
- Keep project terminology based on `MechaFrameLink`.
- Treat `EXIT` in old design documents as the previous project name.

## Version Control

This project intentionally uses Git and Diversion for different types of files.

### Git / GitHub

Git manages text-based project files, including:

- `Source/`
- `Config/`
- `Scripts/`
- `Docs/`
- `.codex/`
- `AGENTS.md`
- `.dvignore`
- `MechaFrameLink.uproject`

Do not add `Content/` to Git.

### Diversion

Diversion manages Unreal Engine binary assets under:

- `Content/**/*.uasset`
- `Content/**/*.umap`

Do not treat Unreal binary assets as normal Git-managed files.

## Unreal Engine / MCP

Use Unreal MCP when Unreal Editor interaction is required.

Examples include:

- creating or modifying Blueprint assets
- creating or modifying Widget Blueprints
- modifying maps or actors
- editing DataAssets
- modifying Unreal asset properties
- inspecting Unreal Editor state

Do not directly modify `.uasset` or `.umap` files as binary files.

For Unreal asset changes:

1. Perform the operation through Unreal Editor / MCP.
2. Let Unreal Editor create or save the asset.
3. Treat the resulting `Content/` changes as Diversion-managed changes.

## C++ and Text Files

C++ and text-based project changes should be made directly in the working tree.

Primary C++ location:

- `Source/MechaFrameLink/`

These changes are versioned through Git.

Prefer inspecting existing architecture and conventions before introducing
new classes or systems.

## Task Completion

Before considering an implementation complete:

1. Verify C++ / text changes.
2. Verify Unreal Editor or MCP changes when applicable.
3. Keep Git-managed and Diversion-managed files separated.
4. Do not commit or push unless explicitly requested.
5. Document meaningful implementations as described below.

## Learning documentation

- After completing a meaningful implementation, add or update a Korean learning note under `Docs/Learning/`.
- Explain the purpose, architecture, execution flow, relevant Unreal concepts, important code, verification steps, and lessons from failed approaches.
- Add every new note to `Docs/Learning/README.md`.
- Keep project terminology based on `MechaFrameLink`; treat `EXIT` in old design documents as the previous project name.