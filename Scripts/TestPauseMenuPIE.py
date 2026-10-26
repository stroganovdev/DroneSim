"""Integration test; restores the pre-test SaveGame and never writes HID mappings."""
import unreal, pathlib, json, time, traceback, math
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
slot=ROOT/'Saved/SaveGames/DroneFlightSettings.sav'
original=slot.read_bytes() if slot.exists() else None
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
started=time.monotonic(); state={'requested':False,'stage':0}; result={}
def finish(ok,error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    result['success']=ok
    if error: result['error']=error
    (ROOT/'Saved/PauseMenuPIE.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    if level.is_in_play_in_editor(): level.editor_request_end_play()
    if original is not None:
        slot.parent.mkdir(parents=True,exist_ok=True); slot.write_bytes(original)
    elif slot.exists(): slot.unlink()
    unreal.log('PAUSE_MENU_TEST '+json.dumps(result))
    unreal.SystemLibrary.quit_editor()
def current(world):
    return unreal.GameplayStatics.get_player_controller(world,0)
def subsystem(world):
    gi=unreal.GameplayStatics.get_game_instance(world)
    return next(o for o in unreal.ObjectIterator(unreal.DroneSettingsSubsystem) if o.get_outer()==gi)
def mass(pc):
    return pc.get_pawn() if False else unreal.GameplayStatics.get_player_pawn(pc,0).get_editor_property('root_component').get_mass()
def tick(dt):
    try:
        elapsed=time.monotonic()-started
        if elapsed>2 and not state['requested']:
            state['requested']=True; level.editor_request_begin_play()
        if not level.is_in_play_in_editor():
            if elapsed>90: finish(False,'PIE did not start')
            return
        world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        pc=current(world); settings=subsystem(world)
        now=unreal.GameplayStatics.get_time_seconds(world)
        stage=state['stage']
        if stage==0 and now>1:
            assert 'BP_DroneTestPlayerController' in pc.get_class().get_name()
            pc.open_pause_menu(); assert pc.is_pause_menu_open()
            assert unreal.GameplayStatics.is_game_paused(world)
            menu=pc.get_pause_menu()
            state.update({'stage':1,'pc':pc,'menu':menu,'wall':elapsed,'time':now,'pawn':unreal.GameplayStatics.get_player_pawn(world,0)})
            state['location']=state['pawn'].get_actor_location()
            state['initial_mass']=settings.get_settings().mass_grams
            settings.set_draft_number('MassGrams',60)
        elif stage==1 and elapsed-state['wall']>0.5:
            assert abs(now-state['time'])<0.001, 'World time advanced during pause'
            loc=state['pawn'].get_actor_location(); old=state['location']
            assert (loc.x-old.x)**2+(loc.y-old.y)**2+(loc.z-old.z)**2<0.001
            state['menu'].resume_flight(); assert not pc.is_pause_menu_open()
            assert not unreal.GameplayStatics.is_game_paused(world)
            assert abs(settings.get_settings().mass_grams-state['initial_mass'])<0.1, 'Cancel applied a draft'
            result['pause_and_cancel']=True
            pc.open_pause_menu(); menu=pc.get_pause_menu()
            for section in range(4): menu.select_section(section)
            menu.select_section(2)
            menu.open_calibration(); assert unreal.GameplayStatics.is_game_paused(world)
            pc.hide_calibration_menu(); assert pc.is_pause_menu_open()
            assert unreal.GameplayStatics.is_game_paused(world)
            result['calibration_returns_to_pause']=True
            settings.set_draft_number('RollRate',800); settings.set_draft_number('CameraFov',110)
            menu.apply_and_resume()
            assert not pc.is_pause_menu_open() and not unreal.GameplayStatics.is_game_paused(world)
            saved=unreal.GameplayStatics.load_game_from_slot('DroneFlightSettings',0); assert saved
            assert saved.get_editor_property('version')==1
            assert abs(saved.get_editor_property('settings').roll_rate-800)<0.1
            pawn=unreal.GameplayStatics.get_player_pawn(world,0)
            assert pawn==state['pawn'], 'Rate change unnecessarily restarted the flight'
            movement=pawn.get_editor_property('movement_component')
            mode=movement.get_active_flight_mode()
            assert abs(mode.get_editor_property('roll_rate_deg_per_second')-800)<0.1
            assert all(abs(c.get_editor_property('field_of_view')-110)<0.1 for c in pawn.get_components_by_class(unreal.CameraComponent))
            result['rates_camera_and_save']=True
            pc.open_pause_menu(); state['new_mass']=55 if abs(settings.get_settings().mass_grams-55)>0.1 else 60
            settings.set_draft_number('MassGrams',state['new_mass'])
            pc.get_pause_menu().apply_and_resume()
            state.update({'stage':2,'wall':elapsed,'old_pawn':pawn})
        elif stage==2 and elapsed-state['wall']>0.4:
            pawn=unreal.GameplayStatics.get_player_pawn(world,0)
            assert pawn and pawn!=state['old_pawn']
            body=pawn.get_editor_property('root_component')
            assert abs(body.get_mass()-state['new_mass']*0.001)<0.001, str(body.get_mass())
            assert len(unreal.GameplayStatics.get_all_actors_of_class(world,pawn.get_class()))==1
            asset=unreal.load_asset('/Game/DroneTest/Meteor75Pro/Frame_Meteor75Pro_Analog')
            assert abs(asset.get_editor_property('mass_kg')-0.01984)<0.00001, 'Original part asset changed'
            result['physical_restart_and_asset_isolation']=True
            pc.open_pause_menu(); menu=pc.get_pause_menu()
            menu.reset_meteor_draft(); menu.apply_and_resume()
            state.update({'stage':3,'wall':elapsed})
        elif stage==3 and elapsed-state['wall']>0.4:
            assert abs(mass(pc)-0.045)<0.001
            assert abs(settings.get_settings().roll_rate-670)<0.1
            result['reset_meteor']=True
            finish(True)
        if elapsed>90: finish(False,'Integration test timed out')
    except Exception: finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
