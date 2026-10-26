"""Verify loading a saved profile into a fresh GameInstance; restore the user's file."""
import unreal,pathlib,json,time,traceback
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
slot=ROOT/'Saved/SaveGames/DroneFlightSettings.sav'
original=slot.read_bytes() if slot.exists() else None
saved=unreal.GameplayStatics.create_save_game_object(unreal.DroneFlightSettingsSave)
values=saved.get_editor_property('settings')
values.set_editor_property('roll_rate',777.0); values.set_editor_property('camera_fov',105.0)
saved.set_editor_property('settings',values)
assert unreal.GameplayStatics.save_game_to_slot(saved,'DroneFlightSettings',0)
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
start=time.monotonic(); state={'requested':False}
def finish(ok,error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    result={'success':ok}
    if error: result['error']=error
    (ROOT/'Saved/PauseMenuPersistence.json').write_text(json.dumps(result,indent=2))
    if level.is_in_play_in_editor(): level.editor_request_end_play()
    if original is not None: slot.write_bytes(original)
    elif slot.exists(): slot.unlink()
    unreal.log('PAUSE_PERSISTENCE_TEST '+json.dumps(result))
    unreal.SystemLibrary.quit_editor()
def tick(dt):
    try:
        elapsed=time.monotonic()-start
        if elapsed>2 and not state['requested']:
            state['requested']=True; level.editor_request_begin_play()
        if level.is_in_play_in_editor():
            world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            if unreal.GameplayStatics.get_time_seconds(world)>1:
                gi=unreal.GameplayStatics.get_game_instance(world)
                settings=next(o for o in unreal.ObjectIterator(unreal.DroneSettingsSubsystem) if o.get_outer()==gi)
                assert abs(settings.get_settings().roll_rate-777)<0.1
                pawn=unreal.GameplayStatics.get_player_pawn(world,0)
                movement=pawn.get_editor_property('movement_component')
                assert abs(movement.get_active_flight_mode().get_editor_property('roll_rate_deg_per_second')-777)<0.1
                assert all(abs(c.get_editor_property('field_of_view')-105)<0.1 for c in pawn.get_components_by_class(unreal.CameraComponent))
                finish(True)
        if elapsed>90: finish(False,'Timed out')
    except Exception: finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
