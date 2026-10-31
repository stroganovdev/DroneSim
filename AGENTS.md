# DroneSimTest: working agreement

For tasks changing project files, read `.agents/skills/drone-quality/SKILL.md` and follow `DevelopmentWorkflow.md`.
Keep this file short; detailed procedures belong in that skill and document.

- Inspect git status/diff before editing. Preserve pre-existing edits and FlightRecord assets.
- Log meaningful changes in `Development/Changes/`: player-visible behavior, architectural decisions, verified results, limitations and devlog material. Do not log every command or create records for ordinary chat/writing requests.
- Classify risk before implementation. Foundational changes (input routing, possession, saves/migrations, physics contracts, packaging or shared interfaces) need two different verification methods before being called verified.
- Always distinguish build success, automated tests and physical/manual gameplay. Never claim equivalence to a real drone without measurements.
- Use the exact Editor build script requested by the user. A running Editor/Live Coding can block it: prepare changes first, preserve user work and ask to close only when necessary.
- No new feature is authorized merely by a journal entry. Keep scope narrow. Do not automatically commit/push/publish from the harness.
- For foundational changes the user authorizes an independent reviewer subagent when available: read-only review, focused context, no nested delegation. Send request/contract/diff/evidence, not the author's preferred verdict. Record findings and resolutions. If unavailable, do an explicitly labelled second self-review with another method; record the lack of independence.
- Journal validation enforces evidence structure, not correctness. If a meaningful check is missing, mark partial/blocked with the reason and backlog follow-up; do not manufacture passing evidence.
