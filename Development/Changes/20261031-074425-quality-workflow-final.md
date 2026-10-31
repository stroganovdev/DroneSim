# Журнал и harness: финальная проверка

ID: 20261031-074425-quality-workflow-final | Risk: foundation | Status: verified

Создать обязательный журнал значимых изменений, материал для девлога и повторную/независимую проверку основ будущих функций.

## acceptance

- Запись не перезаписывает существующий журнал и фиксирует исходное состояние Git
- Провал/пропуск обязательной проверки и изменённый исходник запрещают verified
- Фундаментальные задачи требуют поведения и двух разных ревью
- Процесс сохраняется в репозитории, не меняет пользовательские записи/сохранения

## changes

- Добавлены AGENTS.md, проектный навык и инструкция процесса
- Добавлены создание записи, сборка/тесты с доказательствами, валидация и Markdown-выжимка
- Добавлены отрицательные самотесты и независимое ревью harness

## decisions

- Одна запись на значимую задачу; не журнал каждого клика
- Никакой автоматической публикации, Git hooks или фонового слежения
- Независимый агент использует доступную модель; иной провайдер не подключается
- Завершённый JSON редактируемый, Markdown генерируется; сырьевые логи локальны
- Итерация после 20261031-074100-quality-workflow: замечания ревью исправлены, предыдущие результаты сохранены отдельно

## files

- .gitignore
- AGENTS.md
- .agents/skills/drone-quality/SKILL.md
- DevelopmentWorkflow.md
- Scripts/New-ChangeRecord.ps1
- Scripts/Invoke-QualityHarness.ps1
- Scripts/TestQualityHarness.ps1
- Development/Reviews/quality-workflow-review.md

## limitations

- Структурная валидация не доказывает истинность ручных записей и полноту охвата
- Git hook/CI не установлены, процесс запускается агентом по AGENTS.md или вручную
- Старые задачи не восстановлены задним числом; старые отчёты остаются отдельными
- Реальные полёты, HID и Xbox этим harness автоматически не тестируются
- Штатный quick_validate.py недоступен без PyYAML; метаданные навыка проверены отдельно, зависимости глобально не устанавливались

## contentNotes

- Разработка получила журнал с причинами решений и проверяемыми результатами для девлога
- Независимое ревью обнаружило, что первый валидатор мог пропустить старый зелёный отчёт — добавили хеши исходников
- Проверяем сам процесс проверок отрицательными сценариями, а не обещаем отсутствие багов

## Checks and reviews

- [passed] compiler : Exit=0; build script: Scripts/BuildDroneSimTestEditor.ps1 (evidence: Saved/QualityHarness/20261031-074425-quality-workflow-final/20261031-074429-813/editor-build.log)
- [passed] automation : DroneSimTest suite: passed=3, failed=0, notRun=0 (evidence: Saved/QualityHarness/20261031-074425-quality-workflow-final/20261031-074429-813/Automation/index.json)
- [passed] negative-tests : Ten executable cases passed: rejected missing criteria, stale source, failed/skipped required checks, missing foundation reviews/behavior and tampered evidence; accepted valid/partial fixtures. (evidence: Development/Reviews/quality-workflow-selftests.json)
- [passed] process-failure : Expected failure: missing EngineRoot returned exit1 after successful build; record marked partial. Later normal run passed. (evidence: Saved/QualityHarness/20261031-074100-quality-workflow/20261031-074209-748/automation.log)
- [passed] scope-review : Reviewed instructions/scripts, source hashes, isolated UserDir, Git changes and unchanged nine pre-existing FlightRecords. No game source or user SaveGame edits. (evidence: Development/Reviews/quality-workflow-review.md)
- [not-run] metadata-tool : quick_validate.py attempted but PyYAML unavailable in both bundled Python runtimes; simple frontmatter independently checked, no dependencies installed. (evidence: )
- [passed] metadata-inspection : Verified required name/description frontmatter, UTF8 without BOM, reference paths and AGENTS entrypoint. (evidence: Saved/QualityHarness/SkillFormatCheck.txt)
- [passed] contract-review author-self-review: Inspected failure states, preservation of existing files, truthful devlog boundaries and current source snapshot protocol. (evidence: Development/Reviews/quality-workflow-review.md)
- [passed] consumer-and-failure-review independent-agent:review_quality_workflow: Independent source review completed; earlier findings resolved. Tests were run by author, not reviewer. Same available model, no external provider. (evidence: Development/Reviews/quality-workflow-final-independent.md)

Editable source: 
20261031-074425-quality-workflow-final
.json. Raw Saved evidence is local and not versioned.
