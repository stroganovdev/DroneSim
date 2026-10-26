"""Configure only L_DroneTest, using copies of the plugin's existing sample Blueprints."""
import unreal, json, pathlib, shutil, datetime
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
# Back up the level and its external packages before the first write.
backup=ROOT/'Backup'/'DroneFlightTestBeforeSetup'
if not backup.exists():
    backup.mkdir(parents=True)
    for rel in ['Content/L_DroneTest.umap','Content/__ExternalActors__/L_DroneTest','Content/__ExternalObjects__/L_DroneTest']:
        src=ROOT/rel; dst=backup/rel; dst.parent.mkdir(parents=True,exist_ok=True)
        if src.is_dir(): shutil.copytree(src,dst)
        elif src.exists(): shutil.copy2(src,dst)
world=unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
el=unreal.EditorAssetLibrary
el.make_directory('/Game/DroneTest')
def copy_bp(source,dest):
    if el.does_asset_exist(dest):
        raise RuntimeError('Refusing to overwrite existing asset: '+dest)
    bp=el.duplicate_asset(source,dest)
    assert bp
    return bp
pawn_bp=copy_bp('/DroneSimulator/Blueprints/BP_SampleDronePawn','/Game/DroneTest/BP_DroneTestPawn')
pawn_cls=unreal.load_class(None,'/Game/DroneTest/BP_DroneTestPawn.BP_DroneTestPawn_C')
pawn_cdo=unreal.get_default_object(pawn_cls)
pawn_cdo.set_editor_property('enable_recording',False)
movement=pawn_cdo.get_editor_property('movement_component')
for field,path in {
    'frame_asset':'/DroneSimulator/DroneParts/Frames/Frame_GEPRC_Mark5',
    'motor_asset':'/DroneSimulator/DroneParts/Motors/Motor_GEP_SPEEDX2_1960',
    'battery_asset':'/DroneSimulator/DroneParts/Batteries/Battery_6S_Tattu_1300_v4',
    'propeller_asset':'/DroneSimulator/DroneParts/Props/Prop_Ethix_51x3x3_Simple',
}.items():
    asset=unreal.load_asset(path); assert asset, path
    movement.set_editor_property(field,asset)
angle=unreal.new_object(unreal.FlightModeAngle,outer=movement,name='DroneTestAngle')
angle.set_editor_property('max_roll_angle_deg',25.0)
angle.set_editor_property('max_pitch_angle_deg',25.0)
angle.set_editor_property('yaw_rate_deg_per_second',90.0)
# Retain the sample's race mode so its mode-switch input remains usable.
modes=dict(movement.get_editor_property('flight_modes')); modes['angle']=angle
movement.set_editor_property('flight_modes',modes)
movement.set_editor_property('active_flight_mode','angle')
model=movement.get_editor_property('propulsion_model')
assert model.get_class().get_name()=='PropulsionModelDirectSetpoint'
model.set_editor_property('max_throttle_force',15.0)
model.set_editor_property('max_vertical_speed',5.0)
el.save_loaded_asset(pawn_bp,False)
gm_bp=copy_bp('/DroneSimulator/Blueprints/BP_SampleDroneGamemode','/Game/DroneTest/BP_DroneTestGameMode')
gm_cls=unreal.load_class(None,'/Game/DroneTest/BP_DroneTestGameMode.BP_DroneTestGameMode_C')
gm_cdo=unreal.get_default_object(gm_cls)
gm_cdo.set_editor_property('default_pawn_class',pawn_cls)
controller_cls=unreal.load_class(None,'/DroneSimulator/Blueprints/BP_SampleDronePlayerController.BP_SampleDronePlayerController_C')
gm_cdo.set_editor_property('player_controller_class',controller_cls)
el.save_loaded_asset(gm_bp,False)
world.get_world_settings().set_editor_property('default_game_mode',gm_cls)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
starts=[a for a in actors.get_all_level_actors() if isinstance(a,unreal.PlayerStart)]
assert len(starts)==1
starts[0].modify()
starts[0].set_actor_location(unreal.Vector(0,0,350),False,False)
starts[0].set_actor_rotation(unreal.Rotator(0,0,0),False)
starts[0].set_editor_property('is_spatially_loaded',False)
platform=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(0,0,200))
platform.set_actor_label('DroneTest_LaunchPad')
platform.set_editor_property('is_spatially_loaded',False)
mesh=platform.get_component_by_class(unreal.StaticMeshComponent)
mesh.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
mesh.set_collision_profile_name('BlockAll')
platform.set_actor_scale3d(unreal.Vector(20,20,0.2))
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/L_DroneTest')
assert unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
# Verify the actual spawned components (Blueprint SCS components do not exist on its CDO).
pawn=actors.spawn_actor_from_class(pawn_cls,unreal.Vector(0,0,300))
body=pawn.get_editor_property('root_component')
movement=pawn.get_editor_property('movement_component')
imc=unreal.load_asset('/DroneSimulator/Blueprints/IMC_MappingContext')
report={'game_mode':gm_cls.get_path_name(),'pawn_class':pawn_cls.get_path_name(),'controller_class':controller_cls.get_path_name(),
 'parts':{k:movement.get_editor_property(k).get_path_name() for k in ['frame_asset','motor_asset','battery_asset','propeller_asset']},
 'flight_mode':str(movement.get_editor_property('active_flight_mode')),
 'body':body.get_name(),'simulates_physics':body.is_simulating_physics(),'gravity_enabled':body.is_gravity_enabled(),
 'collision':str(body.get_collision_enabled()),'collision_profile':str(body.get_collision_profile_name()),
 'input_mappings':[{'action':m.get_editor_property('action').get_name(),'key':str(m.get_editor_property('key'))} for m in imc.get_editor_property('default_key_mappings').get_editor_property('mappings')]}
actors.destroy_actor(pawn)
with open(ROOT/'Saved'/'DroneTestSetup.json','w',encoding='utf-8') as f: json.dump(report,f,indent=2)
unreal.log('DRONE_TEST_SETUP_COMPLETE '+json.dumps(report))
