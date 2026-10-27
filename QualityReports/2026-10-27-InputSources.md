# Источники управления — 27 октября 2026

База: c2f07a8. Рабочая папка до задачи была чистой. Резервная копия исходников: Backup/XboxInput_20261027 (локально).

## Изменение

Нормализованный ввод из DroneInputSubsystem теперь направляется через реестр IDroneControlSource. Встроены Radio (прежняя HID-калибровка) и Gamepad (стандартные события Unreal/Slate). Выбор сохраняется в существующем SaveGame; режимы полёта, физика и карта не изменялись. Новые адаптеры регистрируются с уникальным FName, список меню строится по реестру.

Xbox: RT — газ; левый X — yaw; правый Y/X — pitch/roll. Переназначаются четыре оси, мёртвая зона и инверсия pitch. При переключении, переподключении и потере фокуса нужен новый нейтральный ввод; источники не смешиваются, автоматического fallback нет. Подробности и ограничения: ControllerInput.md.

## Доказательства

- Editor build после исправления тестового Outer: Saved/BuildLogs/DroneSimTestEditor_20261027_085716.log, Succeeded, exit0. Перед этим компилировались все изменённые runtime-файлы в успешной сборке 085433.
- Saved/Automation/XboxInputFinal/index.json: 3 succeeded, 0 failed, 0 notRun. Это Settings.Validation, Input.GamepadNormalization, Input.SourceIsolation. Источники в тесте изоляции — независимые искусственные адаптеры; это не физический Xbox.
- Saved/ControlSourcesPIE.json: success=true. Проверены отмена, применение источника без пересоздания Pawn, сохранение LT/deadzone0.21/invertPitch в новом GameInstance, возврат на Radio и сохранение раскладки при Reset Meteor. Запуск с отдельным UserDir QualityXboxPIE; пользовательские настройки не перезаписывались.
- Saved/XboxLegacySave.json: success=true. Копия реального старого SaveGame без ControlSource загрузилась с Radio и раскладкой RT/deadzone0.12. Оригинал не изменён.
- Windows Development package: Saved/BuildLogs/PackageDevelopment_20261027_085921.log, BUILD SUCCESSFUL, exit0, 4m28s. Содержит актуальные препятствия L_DroneTest. Запущен отдельный executable с UserDir QualityXboxPIE.
- Реальный Xbox Wireless Controller отвечает в XInput slot0. В foreground packaged-приложении меню показывает «Xbox / геймпад — подключён», газ0%, вращательные оси0.00 на покое. Интерфейс читаемый; нижние поля доступны прокруткой. Меню открывается Tab. Кнопка View добавлена, но физическое нажатие ещё отдельно не подтверждено.

## Границы

Shipping, несколько одновременно подключённых Xbox и физическое отключение выбранного Xbox во время полёта ещё не проверялись. Возможность HID по-прежнему опирается на существующий профиль Pocket; текущий физический повторный тест Pocket не проводился. Реальная работа осей Xbox и полёт фиксируются ниже после взаимодействия с пользователем.

Известные предупреждения: MSVC14.51 новее рекомендуемого14.50, C4305 в старых UnrealTests.cpp, deprecated ToPaintGeometry в редакторском timeline плагина. Ошибки первой сборки ComboBox исправлены. Первая версия unit-теста использовала неверный Outer; исправлена на GameInstance, окончательный отчёт зелёный. Первое обращение Python к Key исправлено на set_editor_property; итоговый интеграционный тест зелёный.

Финальная проверка интерфейса: исправлена читаемость раскрытых списков (светлый текст на тёмном фоне). Editor: Saved/BuildLogs/DroneSimTestEditor_20261027_091032.log — Succeeded, exit0. Обновление Windows Development с уже приготовленными ассетами: Saved/BuildLogs/XboxStylePackage.log — BUILD SUCCESSFUL, exit0, 83s. В актуальном executable вручную проверено переключение панели Radio/Xbox через список; игра оставлена на паузе в разделе Xbox. После потери фокуса кеш осей очищается: для повторного обнаружения требуется движение стика. Полный физический RT, полёт и View остаются в Q-018.
