# Development workflow

This is a local development harness: project instructions + skill + PowerShell scripts + evidence records.
It is not a background watcher, a guarantee of bug-free code, or a transcript of every action.
New project tasks must read AGENTS.md. A new chat/session may be needed for automatic skill discovery;
AGENTS.md also explicitly points to the skill. No global Codex settings or external AI accounts are changed.

## Start a task
Inspect git status/diff. Existing untracked FlightRecords belong to the user unless proven otherwise.
Run from the project folder:
```powershell
powershell -ExecutionPolicy Bypass -File Scripts/New-ChangeRecord.ps1 -Slug input-switch -Title "Switch input source" -Risk foundation
```
The returned ID names Development/Changes/<ID>.json. Edit that JSON as work progresses.
Record request, acceptance criteria, changes, decisions, exact changed files, limitations and content notes.
The baseline captures HEAD and working-tree status, not a backup. Git/explicit backups still protect files.

Risk: low = prose/cosmetics; normal = isolated behavior; foundation = input, possession, save contracts,
physics, packaging or shared APIs on which future work will depend.
For foundation include contracts, expected extension scenario and failure recovery in acceptance criteria.
Prefer a small extension test over a speculative rewrite.

## Verify
```powershell
powershell -ExecutionPolicy Bypass -File Scripts/Invoke-QualityHarness.ps1 -ChangeId <ID> -Mode Editor
powershell -ExecutionPolicy Bypass -File Scripts/Invoke-QualityHarness.ps1 -ChangeId <ID> -Mode EditorAndTests
powershell -ExecutionPolicy Bypass -File Scripts/Invoke-QualityHarness.ps1 -ChangeId <ID> -Mode Validate
```
Editor calls the existing Scripts/BuildDroneSimTestEditor.ps1 unchanged.
EditorAndTests then runs native DroneSimTest.* tests with UnrealEditor-Cmd and a unique isolated UserDir.
Default EngineRoot is D:/games/UE_5.8; pass -EngineRoot on another machine.
A missing executable, nonzero process, absent/failed/empty test report returns a nonzero exit code.
Raw logs stay in ignored Saved/QualityHarness/<ID>/<run>; summary and SHA256 evidence enter the journal.
Tests of HID, gameplay, menus, migrations, packaging and frame-rate behavior still need their own relevant scenarios.
The harness does not silently launch existing Python PIE scripts (they may create FlightRecords).

checks entries: name, method, status (passed/failed/not-run), evidence (repository-relative file), detail.
review entries: reviewer, method, status (passed/failed), evidence, detail.
Use detailed review notes in Development/Reviews/<ID>-*.md when a review is substantial.
Reviews are conclusions about inspected code, not automatically a second model.
For foundation: at least two different methods across checks/reviews and two passing review passes.
At least one review must inspect contracts/failure paths, another should inspect future-consumer compatibility.
Record whether reviewer is author-self-review or independent-agent:<name>; no invented independent review.

## Close honestly
Set status to verified only when required checks passed. foundation requires two reviews.
partial requires limitations; a failed required check means partial/blocked, never verified.
Any task needs a diff-review check. Build/tests can be not-run for prose-only edits with an explicit reason.
Validate checks structure and referenced files, verifies recorded hashes, and renders a Markdown sibling.
It cannot judge whether prose is truthful or coverage sufficient. It does not block arbitrary git commits:
no Git hooks or CI have been installed. Apply instructions and harness deliberately.

For code/content/config changes, Editor build and behavior-specific checks are required before verified.
Every check records source revision and working-tree status automatically for harness runs; manually added
checks/reviews must describe the inspected scope in detail. Recheck after relevant edits.
Two identical build runs are not independent validation. Another agent may share model biases;
real controller/gameplay tests and measurements remain necessary.

## Devlog
Write brief contentNotes: visible result, an interesting difficulty/decision, an honest limitation,
and optional screenshot/video suggestion. Preserve author attribution: DroneSimulator by Baptiste Hutteau
(Evols), https://github.com/Evols/DroneSimulator. Do not publish or capture private information automatically.
Previous history remains in QualityReports and QualityBacklog; it has not been retroactively reconstructed.

## References
Official conventions: https://learn.chatgpt.com/docs/agent-configuration/agents-md
and https://learn.chatgpt.com/docs/build-skills .
The schema and thresholds here are project conventions, not built-in Codex enforcement.

## Source snapshot and required checks
After implementation and before verification, fill files and run:
```powershell
powershell -ExecutionPolicy Bypass -File Scripts/Invoke-QualityHarness.ps1 -ChangeId <ID> -Mode Snapshot
```
This records SHA256 for each scoped source file (journal siblings excluded to avoid self-reference).
Build/test/validation reject changed or missing files. Snapshot is refused after checks/reviews exist:
preserve the previous record and start a linked iteration when source changes after verification.
Include all relevant changed and dependent contract files; a deliberately omitted path cannot be detected
as relevant by a hash. A baseline is not proof of full coverage.
Checks are required by default. Only explicitly optional checks may set required=false; explain why
and retain the gap in limitations. A required not-run check prevents verified.
Run Scripts/TestQualityHarness.ps1 after changing this process.
