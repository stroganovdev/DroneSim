# Прогулка от третьего лица и переключение на коптер

ID: 20261031-075020-third-person-walk | Risk: foundation | Status: verified

Добавить стандартного персонажа Unreal Third Person для прогулки по текущей карте, с дефолтным управлением клавиатурой и Xbox, сохранив полёт.

## acceptance

- Используются реальные ассеты установленного Epic Third Person без новой карты или перезаписи контента
- Стандартные Move/Look/Jump работают через Enhanced Input
- Переключение сохраняет один коптер и одного персонажа, физика коптера припаркована в прогулке
- Apply/Restart/пауза и повторные переключения не ломают режим; потеря персонажа позволяет вернуться к коптеру
- Возврат к полёту восстанавливает контексты и нейтральный барьер ввода

## changes

- Импортированы 138 стандартных ассетов Epic Third Person без новой карты
- Добавлены прогулка, F/Xbox Menu и кнопка в меню, отдельные Enhanced Input контексты
- Коптер припаркован, ввод изолирован; Apply/reset и потеря персонажа обработаны
- HUD коптера скрывается в прогулке и восстанавливается в полёте
- Добавлены тест интеграции и инструкция, обновлён Windows-билд

## decisions

- Сохранён стандартный Blueprint персонажа Epic, управление клавиатуры не меняется
- Старт остаётся в полёте; F/Xbox Menu и меню паузы переключают режим
- Коптер при прогулке возвращается на исходный старт и отключает симуляцию, вместо полёта без игрока
- Карта L_DroneTest не редактируется; персонаж создаётся в свободном месте у старта

## files

- Source/DroneSimTest/DroneTestPlayerController.h
- Source/DroneSimTest/DroneTestPlayerController.cpp
- Source/DroneSimTest/UI/DronePauseMenu.h
- Source/DroneSimTest/UI/DronePauseMenu.cpp
- Scripts/ImportThirdPersonTemplate.ps1
- Scripts/TestThirdPersonPIE.py
- ThirdPersonWalk.md
- QualityBacklog.md
- Content/Characters/Mannequins/Anims/Death/MM_Death_Back_01.uasset
- Content/Characters/Mannequins/Anims/Death/MM_Death_Front_01.uasset
- Content/Characters/Mannequins/Anims/Death/MM_Death_Front_02.uasset
- Content/Characters/Mannequins/Anims/Death/MM_Death_Front_03.uasset
- Content/Characters/Mannequins/Anims/Death/MM_Death_Left_01.uasset
- Content/Characters/Mannequins/Anims/Death/MM_Death_Right_01.uasset
- Content/Characters/Mannequins/Anims/Pistol/MF_Pistol_Idle_ADS.uasset
- Content/Characters/Mannequins/Anims/Pistol/MM_Pistol_DryFire.uasset
- Content/Characters/Mannequins/Anims/Pistol/MM_Pistol_Equip.uasset
- Content/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage.uasset
- Content/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire.uasset
- Content/Characters/Mannequins/Anims/Pistol/MM_Pistol_Reload.uasset
- Content/Characters/Mannequins/Anims/Pistol/Aim/AO_Pistol.uasset
- Content/Characters/Mannequins/Anims/Pistol/Aim/MF_Pistol_Idle_ADS_AO_CD.uasset
- Content/Characters/Mannequins/Anims/Pistol/Aim/MF_Pistol_Idle_ADS_AO_CU.uasset
- Content/Characters/Mannequins/Anims/Pistol/Aim/MF_Pistol_Idle_ADS1.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Right.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jump/MM_Pistol_Jump_Fall_Loop.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jump/MM_Pistol_Jump_RecoveryAdditive.uasset
- Content/Characters/Mannequins/Anims/Pistol/Jump/MM_Pistol_Jump_Start.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Bwd.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Fwd.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Left.uasset
- Content/Characters/Mannequins/Anims/Pistol/Walk/MF_Pistol_Walk_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/MF_Rifle_Idle_ADS.uasset
- Content/Characters/Mannequins/Anims/Rifle/MM_Rifle_DryFire.uasset
- Content/Characters/Mannequins/Anims/Rifle/MM_Rifle_Equip.uasset
- Content/Characters/Mannequins/Anims/Rifle/MM_Rifle_Fire.uasset
- Content/Characters/Mannequins/Anims/Rifle/MM_Rifle_Reload.uasset
- Content/Characters/Mannequins/Anims/Rifle/AIM/AO_Rifle.uasset
- Content/Characters/Mannequins/Anims/Rifle/AIM/MM_Rifle_Idle_ADS_AO_CC.uasset
- Content/Characters/Mannequins/Anims/Rifle/AIM/MM_Rifle_Idle_ADS_AO_CD.uasset
- Content/Characters/Mannequins/Anims/Rifle/AIM/MM_Rifle_Idle_ADS_AO_CU.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Back_Med_01.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Hvy_01.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_01.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_02.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_03.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_04.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Med_01.uasset
- Content/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Med_02.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Apex.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Fall_Land.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Fall_Loop.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_RecoveryAdditive.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Start_Loop.uasset
- Content/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Start.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Bwd.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Fwd.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Left.uasset
- Content/Characters/Mannequins/Anims/Rifle/Walk/MF_Rifle_Walk_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.uasset
- Content/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run.uasset
- Content/Characters/Mannequins/Anims/Unarmed/MM_Idle.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Attack/MM_ChargedAttack.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jump/MM_Land.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Jump/MM_WallJump.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Right.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Left.uasset
- Content/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Right.uasset
- Content/Characters/Mannequins/Materials/M_Mannequin.uasset
- Content/Characters/Mannequins/Materials/Manny/MI_Manny_01_New.uasset
- Content/Characters/Mannequins/Materials/Manny/MI_Manny_02_New.uasset
- Content/Characters/Mannequins/Materials/Quinn/MI_Quinn_01.uasset
- Content/Characters/Mannequins/Materials/Quinn/MI_Quinn_02.uasset
- Content/Characters/Mannequins/Meshes/SK_Mannequin.uasset
- Content/Characters/Mannequins/Meshes/SKM_Manny_Simple.uasset
- Content/Characters/Mannequins/Meshes/SKM_Quinn_Simple.uasset
- Content/Characters/Mannequins/Rigs/CR_Mannequin_Body.uasset
- Content/Characters/Mannequins/Rigs/CR_Mannequin_FootIK.uasset
- Content/Characters/Mannequins/Rigs/CR_Mannequin_Procedural.uasset
- Content/Characters/Mannequins/Rigs/PA_Mannequin.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_01_BN.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_01_D.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_01_MRA.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_02_BN.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_02_D.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_02_MRA.uasset
- Content/Characters/Mannequins/Textures/Manny/T_Manny_02_N.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_01_D.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_01_MRA.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_01_N.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_02_D.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_02_MRA.uasset
- Content/Characters/Mannequins/Textures/Quinn/T_Quinn_02_N.uasset
- Content/Characters/Mannequins/Textures/Shared/T_UE_Logo_M.uasset
- Content/Input/IMC_Default.uasset
- Content/Input/IMC_MouseLook.uasset
- Content/Input/Actions/IA_Jump.uasset
- Content/Input/Actions/IA_Look.uasset
- Content/Input/Actions/IA_MouseLook.uasset
- Content/Input/Actions/IA_Move.uasset
- Content/Input/Touch/BPI_TouchInterface.uasset
- Content/Input/Touch/UI_Thumbstick.uasset
- Content/Input/Touch/UI_TouchSimple.uasset
- Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset

## limitations

- Реальная сессия Xbox и USB/Bluetooth не проверены; стандартные назначения и Enhanced Input действия проверены
- Shipping, мультиплеер и длительная сессия не проверялись
- Headless PIE использует временный GameStateBase только для исключения старого менеджера записи; packaged проверен с настоящим GameState
- Остались предупреждения MSVC14.51 и deprecated wrapper GetMovementBase в Engine Character.h

## contentNotes

- По текущему полигону теперь можно ходить стандартным манекеном, осматривать препятствия и возвращаться к коптеру
- Независимое ревью нашло проблему рестарта при прогулке: исправили до выпуска, тест считает Pawn после повторных переходов
- Визуальная проверка нашла старый HUD поверх персонажа — теперь он переключается вместе с режимом
- Идея видео: F, прогулка и прыжок между кубами, Tab с кнопкой возврата, F обратно к коптеру

## Checks and reviews

- [passed] compiler : Exit=0; build script: Scripts/BuildDroneSimTestEditor.ps1 (evidence: Saved/QualityHarness/20261031-075020-third-person-walk/20261031-082245-828/editor-build.log)
- [passed] automation : DroneSimTest suite: passed=3, failed=0, notRun=0 (evidence: Saved/QualityHarness/20261031-075020-third-person-walk/20261031-082245-828/Automation/index.json)
- [passed] integration-actions : All assertions passed including template Move/Look/Jump, real key mappings, HUD, repeated transitions, pawn counts, Apply/reset, destroyed walker, pause and no asset writes. Test excludes recorder only via unsaved GameStateBase CDO. (evidence: QualityReports/2026-10-31-ThirdPerson-PIE.json)
- [passed] cook-and-package : Full BuildCookRun succeeded, exit0; existing L_DroneTest and actual character dependencies cooked. (evidence: Saved/BuildLogs/PackageDevelopment_20261031_080809.log)
- [passed] runtime-build : Final HUD code packaged with previously cooked unchanged assets, BUILD SUCCESSFUL exit0. (evidence: Saved/BuildLogs/ThirdPersonFinalPackage.log)
- [passed] manual-ui : Observed character/camera, F roundtrip, Space jump, restored HUD, clean walking HUD, pause-mode label and actual menu switch click. Final executable left paused walking. (evidence: QualityReports/2026-10-31-ThirdPerson.md)
- [passed] log-inspection : Frozen current runtime log has zero Accessed None/LogScript Error/fatal matches after walk/flight/menu transitions. (evidence: Saved/QualityEvidence/ThirdPerson/PackagedFinal-snapshot.log)
- [passed] scope-review : Reviewed source diff and dependencies; no map/GameMode asset edits, imported 138 standard Epic assets unchanged, original nine recordings preserved. (evidence: Development/Reviews/2026-10-31-third-person.md)
- [not-run] hardware-session : Actual Xbox walk session and USB/Bluetooth reconnect not exercised. Default axes/A verified in real mapping context; injected actions and keyboard UI tests passed. (evidence: )
- [passed] contract-and-failure-review author-self-review: Input separation, parking/restoration, reset/apply, failed preflight and destroyed walker inspected and exercised. (evidence: Development/Reviews/2026-10-31-third-person.md)
- [passed] consumer-compatibility-review independent-agent:review_third_person: Separate agent source review, same available model. Three initial findings fixed; final HUD/isolation review found no blocking findings. Agent did not execute tests. (evidence: Development/Reviews/2026-10-31-third-person.md)

Editable source: 20261031-075020-third-person-walk.json. Raw Saved evidence is local and not versioned.
