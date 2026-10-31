"""Third-person integration checks. Run with isolated UserDir QualityWalkPIE; no map/assets are saved."""
import unreal, pathlib, time, json, traceback
ROOT=pathlib.Path(__file__).resolve().parents[1]
def checkpoint(message):
 unreal.log('WALK_TEST: '+message)
 (ROOT/'Saved/ThirdPersonProgress.txt').write_text(message,encoding='utf-8')

assert 'QualityWalkPIE' in unreal.Paths.project_saved_dir(), 'Use isolated -UserDir .../Saved/QualityWalkPIE'
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
# DroneGameState exists solely to spawn an editor recorder. Use base GameState only in this
# unsaved test CDO so assertions do not create FlightRecord assets. Actual packaged game is tested separately.
initial_records=set((ROOT/'Content/FlightRecords').glob('*.uasset'))
editor_world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
mode_defaults=unreal.get_default_object(editor_world.get_world_settings().get_editor_property('default_game_mode'))
original_game_state=mode_defaults.get_editor_property('game_state_class')
mode_defaults.set_editor_property('game_state_class',unreal.GameStateBase)

level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
state={'stage':0,'requested':False}; result={}; started=time.monotonic()
walkmap=unreal.load_asset('/Game/Input/IMC_Default')
mousemap=unreal.load_asset('/Game/Input/IMC_MouseLook')
flightmap=unreal.load_asset('/DroneSimulator/Blueprints/IMC_MappingContext')
move=unreal.load_asset('/Game/Input/Actions/IA_Move')
look=unreal.load_asset('/Game/Input/Actions/IA_MouseLook')
jump=unreal.load_asset('/Game/Input/Actions/IA_Jump')
def mapped(context,mapping):
 value=context.has_mapping_context(mapping)
 # Unreal Python returns the out priority on success (0 is active), None on failure.
 if isinstance(value,tuple): return bool(value[0])
 if isinstance(value,bool): return value
 return value is not None and value>=0
def sub(world,cls):
 gi=unreal.GameplayStatics.get_game_instance(world)
 return next(o for o in unreal.ObjectIterator(cls) if o.get_outer()==gi)
def counts(world):
 return len(unreal.GameplayStatics.get_all_actors_of_class(world,unreal.DronePawn)),len(unreal.GameplayStatics.get_all_actors_of_class(world,unreal.Character))
def advance(stage):
 state.update(stage=stage,at=time.monotonic())
def finish(success,error=''):
 new_records=set((ROOT/'Content/FlightRecords').glob('*.uasset'))-initial_records
 if new_records: success=False; error+=' Unexpected recording assets: '+str(sorted(str(p) for p in new_records))
 result['no_record_assets_created']=not bool(new_records)
 mode_defaults.set_editor_property('game_state_class',original_game_state)
 result.update(success=success,error=error)
 (ROOT/'Saved/ThirdPersonPIE.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
 unreal.unregister_slate_post_tick_callback(handle)
 if level.is_in_play_in_editor(): level.editor_request_end_play()
 state['end']=time.monotonic()
 state['exit_handle']=unreal.register_slate_post_tick_callback(exit_tick)
def exit_tick(dt):
 if not level.is_in_play_in_editor() and time.monotonic()-state['end']>.5:
  unreal.unregister_slate_post_tick_callback(state['exit_handle']); unreal.SystemLibrary.quit_editor()
def tick(dt):
 try:
  if time.monotonic()-started>120: finish(False,'Timeout'); return
  if not state['requested'] and time.monotonic()-started>2:
   state['requested']=True;level.editor_request_begin_play()
  if not level.is_in_play_in_editor(): return
  world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if unreal.GameplayStatics.get_time_seconds(world)<1: return
  pc=unreal.GameplayStatics.get_player_controller(world,0)
  if not pc or not isinstance(pc,unreal.DroneTestPlayerController): return
  inputs=sub(world,unreal.DroneInputSubsystem)
  settings=sub(world,unreal.DroneSettingsSubsystem)
  if state['stage']==0:
   checkpoint('stage0 start'); drone=unreal.GameplayStatics.get_player_pawn(world,0); assert isinstance(drone,unreal.DronePawn)
   drone.set_editor_property('enable_recording',False)
   movement=drone.get_component_by_class(unreal.DroneMovementComponent)
   body=drone.get_editor_property('root_component')
   state.update(drone=drone,movement=movement,body=body,flight_sim=body.is_simulating_physics())
   state['initial_contexts']={o.get_path_name():mapped(o,flightmap) for o in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem)}
   state['hud_visibility']=[(h,h.get_visibility()) for h in unreal.ObjectIterator(unreal.UserWidget) if h.get_owning_player()==pc and h.get_class().get_path_name().startswith('/DroneSimulator/UI/WB_HUD.')]
   result['flight_hud_count']=len(state['hud_visibility'])
   assert result['flight_hud_count']>0, 'Flight HUD missing from test'
   assert not pc.is_walking_mode()
   # Failure preflight must leave the original drone possessed.
   checkpoint('failure preflight'); old_class=pc.get_editor_property('walking_character_class')
   pc.set_editor_property('walking_character_class',None)
   assert not pc.set_walking_mode(True) and unreal.GameplayStatics.get_player_pawn(world,0)==drone
   checkpoint('open pause'); pc.open_pause_menu(); state['error']=pc.get_play_mode_error()
   assert state['error']
   state['old_class']=old_class; advance(1)
  elif state['stage']==1 and time.monotonic()-state['at']>.2:
   assert pc.get_pause_menu().get_status_text()==state['error'], 'Failure vanished from menu'
   pc.set_editor_property('walking_character_class',state['old_class'])
   checkpoint('enter walk'); assert pc.set_walking_mode(True),pc.get_play_mode_error()
   walker=unreal.GameplayStatics.get_player_pawn(world,0);state['walker']=walker
   assert isinstance(walker,unreal.Character)
   assert walker.mesh.get_skeletal_mesh_asset(), 'No mesh'
   assert walker.mesh.get_anim_instance(), 'No animation instance'
   assert not state['body'].is_simulating_physics()
   assert not state['movement'].is_component_tick_enabled()
   assert str(inputs.get_control_source())=='None'
   contexts=next(o for o in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem) if mapped(o,walkmap))
   state['contexts']=contexts
   state['initial_flight_context']=state['initial_contexts'].get(contexts.get_path_name(),False)
   result['initial_flight_context']=state['initial_flight_context']
   assert mapped(contexts,mousemap) and not mapped(contexts,flightmap)
   assert counts(world)==(1,1),counts(world)
   assert all(h.get_visibility()==unreal.SlateVisibility.COLLAPSED for h,v in state['hud_visibility'])
   result['move_keys']=[str(k.get_editor_property('key_name')) for k in contexts.query_keys_mapped_to_action(move)]
   result['jump_keys']=[str(k.get_editor_property('key_name')) for k in contexts.query_keys_mapped_to_action(jump)]
   assert {'W','A','S','D'}.issubset(set(result['move_keys'])),result['move_keys']
   assert 'SpaceBar' in result['jump_keys'] and 'Gamepad_FaceButton_Bottom' in result['jump_keys']
   assert any(k.startswith('Gamepad_') for k in result['move_keys'])
   result['spawn_and_isolation']=True; result['failure_keeps_drone_and_error']=True
   state['start']=walker.get_actor_location(); state['rotation']=pc.get_control_rotation()
   advance(2)
  elif state['stage']==2:
   # Inject through the actual Enhanced Input actions bound by Epic's Blueprint, not AddMovementInput.
   state['contexts'].inject_input_vector_for_action(move,unreal.Vector(0,1,0),[],[])
   state['contexts'].inject_input_vector_for_action(look,unreal.Vector(.5,0,0),[],[])
   if time.monotonic()-state['at']>.7:
    assert state['walker'].get_actor_location().distance(state['start'])>20, 'Move action did not move character'
    assert abs(pc.get_control_rotation().yaw-state['rotation'].yaw)>1, 'Look action did not rotate camera'
    result['enhanced_input_move_and_look']=True
    state['jump_z']=state['walker'].get_actor_location().z
    state['contexts'].inject_input_vector_for_action(jump,unreal.Vector(1,0,0),[],[])
    advance(3)
  elif state['stage']==3:
   if time.monotonic()-state['at']>.15:
    assert state['walker'].get_actor_location().z>state['jump_z']+5, 'Jump action did not raise character'
    result['enhanced_input_jump']=True
    # Physical settings Apply while walking must not respawn the drone or lose walking mode.
    pc.open_pause_menu();settings.set_draft_number('MassGrams',47 if settings.get_settings().mass_grams==46 else 46)
    pc.get_pause_menu().apply_and_resume()
    assert pc.is_walking_mode() and unreal.GameplayStatics.get_player_pawn(world,0)==state['walker']
    assert counts(world)==(1,1) and not state['body'].is_simulating_physics()
    assert str(inputs.get_control_source())=='None'
    pc.restart_flight()
    assert pc.is_walking_mode() and counts(world)==(1,1)
    result['apply_and_reset_while_walking']=True
    for i in range(6):
     assert pc.set_walking_mode(False)
     assert unreal.GameplayStatics.get_player_pawn(world,0)==state['drone']
     assert not mapped(state['contexts'],walkmap)
     assert mapped(state['contexts'],flightmap)==state['initial_flight_context']
     assert state['body'].is_simulating_physics()==state['flight_sim']
     assert inputs.is_waiting_for_neutral()
     assert all(h.get_visibility()==v for h,v in state['hud_visibility'])
     checkpoint('enter walk'); assert pc.set_walking_mode(True),pc.get_play_mode_error()
     assert unreal.GameplayStatics.get_player_pawn(world,0)==state['walker'] and counts(world)==(1,1)
    result['repeated_switch_and_neutral_gate']=True
    # A destroyed walker must still allow returning to the existing drone.
    state['walker'].destroy_actor()
    assert pc.is_walking_mode()
    assert pc.set_walking_mode(False),pc.get_play_mode_error()
    assert unreal.GameplayStatics.get_player_pawn(world,0)==state['drone']
    assert state['body'].is_simulating_physics()==state['flight_sim']
    checkpoint('enter walk'); assert pc.set_walking_mode(True),pc.get_play_mode_error()
    assert counts(world)==(1,1)
    result['destroyed_walker_recovery']=True
    pc.open_pause_menu(); assert unreal.GameplayStatics.is_game_paused(world)
    pc.close_pause_menu(); assert not unreal.GameplayStatics.is_game_paused(world)
    result['pause_in_walk']=True
    assert pc.set_walking_mode(False)
    finish(True)
 except Exception: finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
