# Minimal reusable surface material foundation

ID: 20261102-081209-material-foundation | Risk: foundation | Status: verified

Перенести только мастер, базовый слой и blend из AnomalyLoop, без старых текстур и экземпляров; подготовить папку Instances.

## acceptance

- Only three reusable assets; no AnomalyLoop texture references
- Neutral engine defaults; bounded flatness; readable blend parameters
- Create transient consumer instance and verify compile plus missing-texture recovery
- Existing map and FlightRecords unchanged; Editor build succeeds

## changes

- Added exactly master MM_Backrooms, layer ML_Surface and optional MLB_SurfaceBlend
- Replaced old textures with neutral Engine defaults; flatness bounded0..1 with default0
- Renamed blend parameters, prepared empty Instances folder and authoring guide
- Added disposable consumer/override recovery/two-layer compilation verification

## decisions

- Foundation risk: shared material parameter contract
- Stage only three original packages, duplicate and sanitize in target, remove temporary originals after verification

## files

- Scripts/ImportSurfaceMaterials.py
- Scripts/VerifySurfaceMaterials.py
- SurfaceMaterials.md
- Content/DroneTest/Materials/Instances/.gitkeep
- Content/DroneTest/Materials/Masters/MM_Backrooms.uasset
- Content/DroneTest/Materials/Layers/ML_Surface.uasset
- Content/DroneTest/Materials/Layers/MLB_SurfaceBlend.uasset
- Development/Reviews/2026-11-02-surface-materials.md

## limitations

- No map assignments, Shipping cook, GPU frame-time profiling or user-artwork visual tuning
- Existing MSVC14.51 newer than preferred14.50 warning; test AssetRegistry string lookup deprecated

## contentNotes

- Из собственного AnomalyLoop перенесена только основа материалов, без старых обоев и ковров
- Один лёгкий слой по умолчанию, дополнительные слои подключаются по необходимости
- Тест нового экземпляра обнаружил особенности Unreal API: false от setter проверяем чтением; стек строим целиком
- Готовая основа для собственных текстур; визуальный стиль уровня пока не изменён

## Checks and reviews

- [passed] compiler : Exit=0; build script: Scripts/BuildDroneSimTestEditor.ps1 (evidence: Saved/QualityHarness/20261102-081209-material-foundation/20261102-082118-650/editor-build.log)
- [passed] dx12-sm6-consumer-and-recovery : Final verification after source snapshot: exit0; three foundation assets loaded without old packages; valid stack/root, instance readback, Tiling reset2 to1, optional two-layer compile; no saved test assets. Raw Saved/Logs/VerifySurfaceMaterials-Final.log. (evidence: QualityReports/2026-11-02-SurfaceMaterials.json)
- [passed] scope-and-dependency-review : All added implementation files match scoped snapshot. Existing tracked files/map untouched,13original FlightRecords still untracked; source AnomalyLoop git clean. (evidence: Development/Reviews/2026-11-02-surface-materials.md)
- [passed] graph-contract-and-failure-inspection author-self-review: Inspected topology, bounded flatness, neutral defaults, saved metadata and foreign dependency closure against accepted minimal contract. (evidence: Development/Reviews/2026-11-02-surface-materials.md)
- [passed] independent-future-consumer-review independent-agent:/root/review_material_foundation: Same-model read-only reviewer inspected source and execution evidence; stack atomicity, future Instances and observable recovery findings resolved; no remaining blockers. (evidence: Development/Reviews/2026-11-02-surface-materials.md)

Editable source: 20261102-081209-material-foundation.json. Raw Saved evidence is local and not versioned.
