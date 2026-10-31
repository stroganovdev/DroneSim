"""Verify saved geometry and real game-world collision. Run with isolated QualityYardVerify UserDir."""
import unreal,pathlib,json,time,traceback
ROOT=pathlib.Path(__file__).resolve().parents[1]
assert 'QualityYardVerify' in unreal.Paths.project_saved_dir(),'Use isolated QualityYardVerify UserDir'
manifest=ROOT/'Saved/FlightYardSetup.json'
if not manifest.exists():manifest=ROOT/'QualityReports/2026-10-31-FlightYard-Setup.json'
setup=json.loads(manifest.read_text())
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
editor_world=unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
yard={a.get_actor_label():a for a in actors if a.get_actor_label().startswith('FlightYard_')}
assert len(yard)==setup['new_actor_count']
assert sum(a.get_actor_label().startswith('DroneCourse_') for a in actors)==34
for part in setup['parts']:
 actor=yard[part['name']]
 p=actor.get_actor_location();s=actor.get_actor_scale3d()
 assert max(abs(v-w) for v,w in zip([p.x,p.y,p.z],part['center']))<.1
 assert max(abs(v*100-w) for v,w in zip([s.x,s.y,s.z],part['size']))<.1
 component=actor.get_component_by_class(unreal.StaticMeshComponent)
 assert str(component.get_collision_profile_name())=='BlockAll' and not component.is_simulating_physics()
assert setup['stair_step_height_cm']<45 and setup['stair_tread_cm']>=42
# Isolate the plugin's editor recorder only; no map/default asset is saved.
mode_defaults=unreal.get_default_object(editor_world.get_world_settings().get_editor_property('default_game_mode'))
original_state=mode_defaults.get_editor_property('game_state_class')
mode_defaults.set_editor_property('game_state_class',unreal.GameStateBase)
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started=time.monotonic();state={'requested':False};report={}
output=ROOT/'Saved/FlightYardVerification.json'
output.write_text(json.dumps({'success':False,'status':'running'}))
def blocking(value):
 if value is None:return False
 if isinstance(value,tuple):
  for v in value:
   if isinstance(v,bool):return v
  value=value[0]
 # SphereTraceSingle has one out-HitResult and a success bool: Python returns HitResult on success, None on failure.
 assert isinstance(value,unreal.HitResult),type(value)
 return True
def sweep(world,a,b):
 return unreal.SystemLibrary.sphere_trace_single(world,unreal.Vector(*a),unreal.Vector(*b),45,
  unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,False,[],unreal.DrawDebugTrace.NONE,True)
def finish(success,error=''):
 report.update(success=success,error=error)
 output.write_text(json.dumps(report,indent=2),encoding='utf-8')
 mode_defaults.set_editor_property('game_state_class',original_state)
 unreal.unregister_slate_post_tick_callback(handle)
 if level.is_in_play_in_editor():level.editor_request_end_play()
 state['end']=time.monotonic()
 state['exit_handle']=unreal.register_slate_post_tick_callback(exit_tick)
def exit_tick(dt):
 if not level.is_in_play_in_editor() and time.monotonic()-state['end']>.5:
  unreal.unregister_slate_post_tick_callback(state['exit_handle']);unreal.SystemLibrary.quit_editor()
def tick(dt):
 try:
  if time.monotonic()-started>90:finish(False,'PIE timeout');return
  if not state['requested'] and time.monotonic()-started>2:
   state['requested']=True;level.editor_request_begin_play()
  if not level.is_in_play_in_editor():return
  world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if unreal.GameplayStatics.get_time_seconds(world)<1:return
  count=sum(a.actor_has_tag('DroneFlightYard') for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.StaticMeshActor))
  assert count==setup['new_actor_count'],count
  control=sweep(world,[3100,-3800,350],[3100,-3000,350])
  unreal.log('YARD_CONTROL '+str(control))
  assert blocking(control),'Collision query missed the known solid house wall'
  checks=[]
  for name,points in setup['routes'].items():
   for i,(a,b) in enumerate(zip(points,points[1:])):
    hit=sweep(world,a,b)
    assert not blocking(hit),(name,i,str(hit))
    checks.append({'route':name,'segment':i,'clear':True})
  report.update(reloaded_actor_count=count,old_course_actors=34,collision_profiles='BlockAll',
   sphere_radius_cm=45,solid_wall_control_hit=True,physics_world='PIE',physics_sweeps=checks,
   stair_step_geometry_compatible_with_default_character=True,manual_flight_test=False,
   recorder_excluded_in_unsaved_test_cdo=True)
  unreal.log('FLIGHT_YARD_VERIFIED_GAME_WORLD '+str(len(checks))+' sweeps')
  finish(True)
 except Exception:finish(False,traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
