---
name: drone-quality
description: Record and verify meaningful DroneSimTest project changes with evidence and devlog notes. Use for project implementation and fixes, especially shared foundations; not ordinary discussion or post rewriting.
---
Read `DevelopmentWorkflow.md` at the repository root. Paths below are repository-relative.

Before editing, inspect existing changes and create one record with `Scripts/New-ChangeRecord.ps1`. Group one coherent task, not individual commands. Choose low/normal/foundation risk from actual dependencies.

Fill acceptance criteria and affected contracts before implementation. For foundation risk, verify a consumer/extension scenario and a failure/recovery scenario as relevant. Choose a focused independent reviewer subagent if available and authorized by AGENTS.md; otherwise clearly label a second self-review. Two identical build runs do not count as two methods.

Use `Scripts/Invoke-QualityHarness.ps1` to build/run native tests and attach evidence. Run focused PIE/manual scenarios when required by the behavior. The harness never opens physical devices or runs PIE asset-generating scripts automatically. Test profiles belong under isolated Saved paths.

Complete JSON notes: what changed, why it matters, decisions, file scope, limitations, checks and review findings. A check has a method, actual outcome and evidence path; passed requires a file that exists. Include source revision/scope to avoid passing an old report as current evidence.

Validate/render with `-Mode Validate`. Fix genuine gaps; mark partial when required verification is unavailable. Generated Markdown is the readable devlog source, JSON is the editable record. Historical summaries must be labelled reconstructed and link old reports, never invented command histories.
