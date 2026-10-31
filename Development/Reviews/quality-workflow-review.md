# Quality workflow: independent review and resolution
Scope: new harness, New-ChangeRecord, self-tests, AGENTS, skill, workflow and gitignore.
Reviewer: independent-agent:review_quality_workflow, separate context execution using the same available model.
This is independent agent review, not a different model/provider.

First pass found missing/null arrays, skipped checks, stale source evidence, Windows path normalization,
malformed test counters and omitted reviewer attribution. All were corrected in the implementation.
Second pass confirmed those fixes and found deleted-file handling plus minor bool/exit-code issues.
Final corrections: tracked deleted paths use <deleted> fingerprint, Boolean counters are rejected,
native exit codes are reset and missing exit code fails. Execution tests are reported separately.
No review claims to prove gameplay correctness or full coverage.

Author contract review:
- User FlightRecords, saves, HID calibration and gameplay code are not edited.
- Missing/failed/skipped required checks cannot produce verified.
- Scope fingerprints reject changed content; scoped tracked deletions are supported.
- JSON records do not imply remote publication or Git authorization.
- Manual scope and review truthfulness remain a human/agent responsibility.
Final independent follow-up: staged deletions must also resolve against baseline revision; implemented Git baseline-tree fallback. Manual source scope and real gameplay coverage remain limitations.
