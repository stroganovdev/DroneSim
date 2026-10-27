"""Integration test using an isolated -UserDir; does not modify existing assets or normal user saves."""
import unreal,pathlib,time,json,traceback
ROOT=pathlib.Path(__file__).resolve().parents[1]
assert 'QualityXboxPIE' in unreal.Paths.project_saved_dir(), 'Run with isolated -UserDir=.../Saved/QualityXboxPIE'
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
start=time.monotonic(); state={'stage':0,'requested':False}; result={}
def sub(world,cls):
 gi=unreal.GameplayStatics.get_game_instance(world)
 return next(o for o in unreal.ObjectIterator(cls) if o.get_outer()==gi)
def finish(success,error=''):
 result.update(success=success,error=error)
 (ROOT/'Saved/ControlSourcesPIE.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
 unreal.unregister_slate_post_tick_callback(handle)
 if level.is_in_play_in_editor(): level.editor_request_end_play()
 state['finished']=True
 # Let the asynchronous EndPlay finish before requesting editor exit.
 state['finish_wall']=time.monotonic()
 state['exit_handle']=unreal.register_slate_post_tick_callback(exit_tick)
def exit_tick(dt):
 if not level.is_in_play_in_editor() and time.monotonic()-state['finish_wall']>.5:
  unreal.unregister_slate_post_tick_callback(state['exit_handle']); unreal.SystemLibrary.quit_editor()
def tick(dt):
 try:
  now=time.monotonic()-start
  if now>2 and not state['requested']:
   state['requested']=True; level.editor_request_begin_play()
  if state['stage']==1 and not level.is_in_play_in_editor():
   if time.monotonic()-state['ended']>1:
    level.editor_request_begin_play(); state['stage']=2
   return
  if not level.is_in_play_in_editor():
   if now>100: finish(False,'PIE did not start')
   return
  world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if unreal.GameplayStatics.get_time_seconds(world)<1: return
  pc=unreal.GameplayStatics.get_player_controller(world,0)
  pawn=unreal.GameplayStatics.get_player_pawn(world,0)
  settings=sub(world,unreal.DroneSettingsSubsystem); inputs=sub(world,unreal.DroneInputSubsystem)
  if state['stage']==0:
   options={str(o.id) for o in inputs.get_control_sources()}
   assert {'Radio','Gamepad'}.issubset(options)
   original=str(settings.get_settings().control_source)
   pc.open_pause_menu(); settings.set_draft_control_source('Gamepad'); pc.get_pause_menu().select_section(2)
   assert str(inputs.get_control_source())==original, 'Draft changed live source'
   pc.get_pause_menu().resume_flight()
   assert str(settings.get_settings().control_source)==original
   result['draft_cancel']=True
   pc.open_pause_menu(); settings.set_draft_control_source('Gamepad')
   layout=settings.get_draft().gamepad; layout.deadzone=.21; layout.invert_pitch=True
   key=unreal.Key(); key.set_editor_property('key_name','Gamepad_LeftTriggerAxis')
   layout.throttle=key; settings.set_draft_gamepad(layout)
   pc.get_pause_menu().select_section(2); pc.get_pause_menu().apply_and_resume()
   assert str(inputs.get_control_source())=='Gamepad'
   assert unreal.GameplayStatics.get_player_pawn(world,0)==pawn, 'Source switch restarted aircraft'
   pc.open_pause_menu(); pc.get_pause_menu().select_section(2)
   assert unreal.GameplayStatics.is_game_paused(world)
   settings.reset_draft()
   assert str(settings.get_draft().control_source)=='Gamepad'
   assert abs(settings.get_draft().gamepad.deadzone-.21)<.001
   assert str(settings.get_draft().gamepad.throttle.get_editor_property('key_name'))=='Gamepad_LeftTriggerAxis'
   result['apply_without_restart_and_independent_reset']=True
   pc.close_pause_menu(); level.editor_request_end_play(); state.update(stage=1,ended=time.monotonic())
  elif state['stage']==2:
   current=settings.get_settings()
   assert str(current.control_source)=='Gamepad'
   assert abs(current.gamepad.deadzone-.21)<.001 and current.gamepad.invert_pitch
   assert str(current.gamepad.throttle.get_editor_property('key_name'))=='Gamepad_LeftTriggerAxis'
   assert str(inputs.get_control_source())=='Gamepad'
   result['new_game_instance_persistence']=True
   pc.open_pause_menu(); settings.set_draft_control_source('Radio'); pc.get_pause_menu().apply_and_resume()
   assert str(inputs.get_control_source())=='Radio'
   assert unreal.GameplayStatics.get_player_pawn(world,0)==pawn
   result['switch_back_to_radio']=True
   pc.open_pause_menu(); settings.set_draft_control_source('Gamepad'); settings.set_draft_gamepad(unreal.DroneGamepadSettings())
   pc.get_pause_menu().apply_and_resume(); pc.open_pause_menu(); pc.get_pause_menu().select_section(2)
   finish(True)
  if now>100: finish(False,'Timed out')
 except Exception: finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
