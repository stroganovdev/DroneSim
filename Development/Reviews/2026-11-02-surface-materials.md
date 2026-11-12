# Material foundation review — 2026-11-02

Scope: baseline b592b92; Scripts/ImportSurfaceMaterials.py, VerifySurfaceMaterials.py, SurfaceMaterials.md and three new assets under /Game/DroneTest/Materials. No changes to tracked baseline files, map or source AnomalyLoop;13user untracked recordings preserved.

## Author self-review: contract and failure paths
Method: inspect imported graph topology, saved metadata, asset dependency closure; compare file scope with baseline. One layer is connected to MP_MATERIAL_ATTRIBUTES. Default textures are Engine WhiteSquareTexture and DefaultNormal. Normal_Flatness is0 by default and passes Saturate before FlattenNormal. Roughness retains Clamp0..1. BlendMask_Texture and Blend_Weight replace Param and1; Mask_Intensity0 avoids a white mask forcing an overlay. Import refuses existing destination assets and saves only after compile success. Test consumers are not saved. Original three staging packages were hash-compared to source before exact-file removal; no recursive cleanup of user assets.
Result: passed for scoped shared material contract. Engine function preview HeatmapGradient remains an Engine dependency, not a copied user texture.

## Independent review: future consumer and verification design
Reviewer: independent-agent:/root/review_material_foundation (same model, read-only; no nested agents or independent execution).
Inspected source/scripts/docs, binary package paths, engine invariants and executed logs.
Findings resolved:
- Live struct modification in two-layer test caused invalid intermediate counts and an assertion. Detached complete struct assigned once now.
- Restrict tests to foundation paths rather than forbidding future Instances under root.
- Recovery now verifies observable Tiling2→1 instead of only same-default normal texture.
- GuidLibrary.new_guid replaces unsupported positional Guid constructor.
Final result: no remaining blocking findings. VerifySurfaceMaterials-4.log completed successfully, saved master metadata/root, override readback/recovery, foreign-dependency rejection and two-layer compile passed.
Nonblocking: string AssetRegistry.get_asset_by_object_path deprecated in test; material assets unaffected.

## Evidence and limits
QualityReports/2026-11-02-SurfaceMaterials.json; Saved/Logs/VerifySurfaceMaterials-4.log; exact Editor build Saved/BuildLogs/DroneSimTestEditor_20261102_081353.log succeeded. Final harness build follows snapshot.
This is not a Shipping cook or FPS measurement. No materials assigned to game map. User-texture artwork and visual tuning are future work.
