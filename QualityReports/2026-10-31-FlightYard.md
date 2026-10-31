# Flight yard — 31 October 2026

Implemented on the existing L_DroneTest: 222 static mesh actors, using the existing Engine cube and course materials.
Two-storey training house with large unglazed windows, open-riser internal stair, top landing and roof skylight.
Scaffold tower with two ring decks, vertical passage and decorative external ladder.
Three polygon rings at different heights. Original34course actors and loaded pre-existing poses/GameMode preserved.
Backup before saved generation: see QualityReports/2026-10-31-FlightYard-Setup.json.

## Evidence and interpretation
- Generator sampled intended routes against all newly built oriented boxes with45cm clearance radius.
  An initial pre-save failure detected the lower-exit route touching a handrail; route and upper landing corrected.
- Saved geometry was reloaded and checked against the manifest for actor count, position, scale and BlockAll.
- Important test correction: commandlet/editor-world collision queries missed a known solid wall.
  Those initial empty trace results are not accepted as real physics evidence.
  Final verification runs in PIE, requires a hit on a known wall, then requires all13intended segments clear.
  Python SphereTraceSingle returns HitResult on success and None on failure; corrected test adapter.
- For test isolation only, an unsaved GameMode CDO uses base GameState rather than the recorder-spawning
  DroneGameState; original default restored. The actual packaged build uses the original GameState.
- Two actual editor renders inspected: Saved/Screenshots/FlightYardOverview.png and FlightHouseStairs.png.
- Editor build: Saved/BuildLogs/DroneSimTestEditor_20261031_083755.log — Succeeded, exit0.
- Windows full BuildCookRun: Saved/BuildLogs/PackageDevelopment_20261031_084223.log — BUILD SUCCESSFUL,
  exit0, 132.50s. Current executable launched,222DroneFlightYard tag lines found in actual runtime log.
- Physics/packaged summaries: QualityReports/2026-10-31-FlightYard-Verification.json and -Runtime.json.
- Final record adds the latest Editor build and raw log hashes.

## Limits
No manual RC flight through the house or physical character climb of the staircase was performed.
Step geometry (rise19.57cm, tread50cm) is compatible with default Character constraints, not a traversal test.
Tower ladder is static geometry without a climbing interaction.
No FPS/long-session measurements; Shipping not built.
Existing toolchain preference warning MSVC14.51 versus14.50 remains.
