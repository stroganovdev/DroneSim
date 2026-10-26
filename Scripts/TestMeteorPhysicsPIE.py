"""PIE takeoff regression using temporary motor commands; never writes input profiles/assets."""
import unreal, json, pathlib, time, traceback
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
start=time.monotonic(); state={'requested':False,'stage':0}; report={}
def finish(ok,error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    report['success']=ok
    if error: report['error']=error
    (ROOT/'Saved/MeteorPhysicsPIE.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    unreal.log('METEOR_PHYSICS_PIE '+json.dumps(report))
    if level.is_in_play_in_editor(): level.editor_request_end_play()
    unreal.SystemLibrary.quit_editor()
def tick(dt):
    try:
        elapsed=time.monotonic()-start
        if not state['requested'] and elapsed>2:
            level.editor_request_begin_play(); state['requested']=True
        if level.is_in_play_in_editor():
            world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            now=unreal.GameplayStatics.get_time_seconds(world)
            if state['stage']==0 and now>1.5:
                pc=unreal.GameplayStatics.get_player_controller(world,0)
                pawn=unreal.GameplayStatics.get_player_pawn(world,0); assert pawn
                body=pawn.get_editor_property('root_component')
                movement=pawn.get_editor_property('movement_component')
                model=movement.get_editor_property('propulsion_model')
                inputs=next(o for o in unreal.ObjectIterator(unreal.DroneInputSubsystem) if o.get_outer()==unreal.GameplayStatics.get_game_instance(world))
                report.update({'pawn':pawn.get_class().get_path_name(),'pawn_count':len(unreal.GameplayStatics.get_all_actors_of_class(world,pawn.get_class())),
                    'mass_kg':body.get_mass(),'inertia_kg_cm2':str(body.get_inertia_tensor()),'model':model.get_class().get_name(),
                    'body_physics':body.is_simulating_physics(),'movement_active':movement.is_active(),'movement_tick':movement.is_component_tick_enabled(),
                    'updated_component':str(movement.get_editor_property('updated_component')),
                    'player_input':str(movement.get_editor_property('player_input')),'setpoint':str(movement.get_editor_property('setpoint')),
                    'body_instance':str(body.get_editor_property('body_instance')),
                    'motor_response_seconds':model.get_editor_property('motor_response_time_seconds'),
                    'mode':str(movement.get_editor_property('active_flight_mode')),
                    'parts':{n:movement.get_editor_property(n).get_path_name() for n in ['frame_asset','motor_asset','battery_asset','propeller_asset']},
                    'pocket_calibrated':any('pocket' in d.device_name.lower() and inputs.is_device_calibrated(d.device_id) for d in inputs.get_all_devices())})
                assert report['pawn_count']==1 and report['model']=='PropulsionModelDynamics'
                assert abs(report['mass_kg']-0.045)<0.001
                inertia=body.get_inertia_tensor()
                assert abs(inertia.x-0.21)<0.01 and abs(inertia.y-0.25)<0.01 and abs(inertia.z-0.42)<0.01
                assert all('/Meteor75Pro/' in p for p in report['parts'].values())
                # Eliminate live stick motion for a deterministic vertical test.
                pc.un_possess()
                body.set_world_location_and_rotation(unreal.Vector(0,0,240),unreal.Rotator(0,0,0),False,True)
                body.set_physics_linear_velocity(unreal.Vector(0,0,0),False)
                body.set_physics_angular_velocity_in_radians(unreal.Vector(0,0,0),False)
                pid=model.get_editor_property('drone_controller')
                pid.set_editor_property('min_throttle',0.05)
                state.update({'stage':1,'pawn':pawn,'body':body,'pid':pid,'phase_start':now})
            elif state['stage']==1 and now-state['phase_start']>0.7:
                body=state['body']; state['start_z']=body.get_world_location().z
                report['rest_z_cm']=state['start_z']
                assert state['start_z']>205 and state['start_z']<220, 'Contact with launch pad failed'
                state['pid'].set_editor_property('min_throttle',0.6)
                state['phase_start']=now; state['stage']=2
            elif state['stage']==2 and now-state['phase_start']>=1.0:
                body=state['body']; dz=(body.get_world_location().z-state['start_z'])/100
                vz=body.get_physics_linear_velocity().z/100
                report.update({'takeoff_throttle':0.6,'takeoff_duration_s':now-state['phase_start'],'height_gain_m':dz,'vertical_speed_m_s':vz,
                    'angular_speed_rad_s':str(body.get_physics_angular_velocity_in_radians())})
                assert dz>2.0, '60 percent throttle did not produce prompt takeoff'
                assert vz>5.0, 'Unexpected vertical speed cap or insufficient thrust'
                omega=body.get_physics_angular_velocity_in_radians()
                assert omega.x*omega.x+omega.y*omega.y+omega.z*omega.z<1.0, 'Vertical climb unstable'
                finish(True)
        if elapsed>70: finish(False,'Timed out')
    except Exception: finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
