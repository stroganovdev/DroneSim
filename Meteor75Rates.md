# Meteor75 Pro: чувствительность стиков

> Физическая модель также обновлена после этой настройки: см. Meteor75Physics.md. Описание сохранённой прежней физики ниже относится к предыдущему этапу.

В BP_DroneTestPawn выбран существующий режим `race` (UFlightModeAir / Acro). Стандартная раскладка Pocket Mode 2 / AETR сохранена. Мастер калибровки не нужен.

Actual Rates для roll, pitch и yaw: Center Sensitivity 70, Max Rate 670°/с, Expo 0. Воспроизведена нелинейная кривая Betaflight, а не линейное умножение стика на максимальную скорость. Контрольные значения: 25% стика → 55°/с, 50% → 185°/с, 100% → 670°/с. Газ и его калибровка не менялись.

Источник: заводской CLI аналогового Meteor75 Pro ELRS / Matrix / Betaflight 4.5.3 (2026): https://support.betafpv.com/hc/article_attachments/55526741863833 . CLI не переопределяет rateprofile, поэтому используются defaults соответствующего Betaflight: https://github.com/betaflight/betaflight/blob/4.5.3/src/main/fc/controlrate_profile.c . Формула: applyActualRates в https://github.com/betaflight/betaflight/blob/4.5.3/src/main/fc/rc.c . Это не извлечение пресета из Liftoff Micro Drones 1.1.2 и не копирование массы, моторов, PID и всей физики реального whoop.

Откройте проект и нажмите Play на L_DroneTest. Газ перед запуском уберите вниз. В Acro коптер сохраняет наклон после отпускания правого стика; автоматического выравнивания нет. M переключает существующие режимы, включая оставленный `angle`. V — камера, R — перезапуск, Esc — остановка.

Исходники FlightModeAir.h/.cpp: добавлен выключенный по умолчанию переключатель use_actual_rates и параметры центра/Expo. Старые Air-ассеты продолжают работать с прежней линейной кривой; Actual включён только на проектном тестовом Pawn. FlightModeAirTests.cpp проверяет знаки всех трёх осей, нейтраль, промежуточные положения и максимум, Expo, сохранение газа и старого линейного режима. Scripts/ConfigureMeteor75Rates.py применяет профиль к существующему тестовому Pawn. Scripts/TestDroneFlightPIE.py проверяет его загрузку с настоящим HID Pocket.

Резервные копии исходного Pawn и FlightModeAir: Backup/Meteor75Rates_20261025_132812.

Проверки завершены успешно: DroneSimulator.Input.Meteor75ActualRates — 1 тест, 0 ошибок; PIE — race/Actual, 670°/с по трём осям, один управляемый Pawn, профиль Pocket загружен. Отчёты: Saved/Automation/Meteor75Rates/index.json и Saved/Meteor75PIE.json. Автоматический PIE выполнялся без рендера (NullRHI).
