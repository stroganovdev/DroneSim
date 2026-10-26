"""PIE smoke test; does not write HID mappings or synthesize controller calibration."""
import unreal, time, json, pathlib, traceback
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
started=time.monotonic(); state={'requested':False,'checked':False}; report={}
def finish(ok,error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    report['success']=ok
    if error: report['error']=error
    with open(ROOT/'Saved'/'DroneTestPIE.json','w',encoding='utf-8') as f: json.dump(report,f,indent=2)
    unreal.log('DRONE_PIE_RESULT '+json.dumps(report))
    if level.is_in_play_in_editor(): level.editor_request_end_play()
    unreal.SystemLibrary.quit_editor()
def tick(dt):
    try:
        elapsed=time.monotonic()-started
        if not state['requested'] and elapsed>2:
            level.editor_request_begin_play(); state['requested']=True
        if level.is_in_play_in_editor() and not state['checked'] and elapsed>10:
            world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            assert world, 'No PIE game world'
            pc=unreal.GameplayStatics.get_player_controller(world,0); assert pc
            pawn=unreal.GameplayStatics.get_player_pawn(world,0); assert pawn
            gm=unreal.GameplayStatics.get_game_mode(world)
            movement=pawn.get_editor_property('movement_component'); body=pawn.get_editor_property('root_component')
            inputs=next(o for o in unreal.ObjectIterator(unreal.DroneInputSubsystem) if o.get_outer()==unreal.GameplayStatics.get_game_instance(world))
            devices=inputs.get_all_devices()
            report.update({'game_mode':gm.get_class().get_path_name(),'controller':pc.get_class().get_path_name(),'pawn':pawn.get_class().get_path_name(),
                'pawn_count':len(unreal.GameplayStatics.get_all_actors_of_class(world,pawn.get_class())),
                'location':str(pawn.get_actor_location()),'updated_component':str(movement.get_editor_property('updated_component')),
                'active_mode':str(movement.get_editor_property('active_flight_mode')),
                'simulates_physics':body.is_simulating_physics(),'gravity_enabled':body.is_gravity_enabled(),
                'devices':[{'name':d.device_name,'uid':d.device_uid,'axes':{str(k):v for k,v in d.raw_axes.items()},'calibrated':inputs.is_device_calibrated(d.device_id)} for d in devices]})
            assert 'BP_DroneTestPawn' in report['pawn']
            assert report['pawn_count']==1
            assert report['active_mode']=='race'
            air=movement.get_active_flight_mode()
            report['actual_rates']={'enabled':air.get_editor_property('use_actual_rates'),
                'center':str(air.get_editor_property('actual_center_sensitivity')),
                'expo':str(air.get_editor_property('actual_expo')),
                'maximum':[air.get_editor_property(n) for n in ['roll_rate_deg_per_second','pitch_rate_deg_per_second','yaw_rate_deg_per_second']]}
            assert report['actual_rates']['enabled']
            assert report['actual_rates']['maximum']==[670.0,670.0,670.0]
            assert any('pocket' in d.device_name.lower() and inputs.is_device_calibrated(d.device_id) for d in devices)
            assert movement.get_editor_property('updated_component')==body
            assert report['simulates_physics'] and not report['gravity_enabled']
            assert pawn.get_actor_location().z>180, 'Pawn fell through launch pad'
            assert any('pocket' in d.device_name.lower() and len(d.raw_axes)>=4 for d in devices), 'Pocket did not deliver HID reports'
            imc=unreal.load_asset('/DroneSimulator/Blueprints/IMC_MappingContext')
            report['keys']=[{'action':m.action.get_name(),'key':str(unreal.InputLibrary.key_get_display_name(m.key))} for m in imc.get_editor_property('default_key_mappings').mappings]
            pc.show_calibration_menu(); assert pc.get_editor_property('show_mouse_cursor')
            pc.hide_calibration_menu(); assert not pc.get_editor_property('show_mouse_cursor')
            report['calibration_menu_open_close']=True
            state['checked']=True
        if state['checked'] and elapsed>12:
            finish(True)
        elif elapsed>60: finish(False,'Timed out waiting for PIE')
    except Exception:
        finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
