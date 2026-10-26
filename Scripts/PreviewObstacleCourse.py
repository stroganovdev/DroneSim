"""Reload and render the saved obstacle course from an editor overview; no PIE or map saves."""
import unreal,time,pathlib,json
ROOT=pathlib.Path(__file__).resolve().parents[1]
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
world=unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
course=[a for a in actors.get_all_level_actors() if a.get_actor_label().startswith('DroneCourse_')]
assert len(course)==34
for a in course:
    assert str(a.get_component_by_class(unreal.StaticMeshComponent).get_collision_profile_name())=='BlockAll'
    r=a.get_actor_rotation()
    assert abs(r.pitch)+abs(r.roll)<0.01, 'Obstacle is tilted: '+a.get_actor_label()
position=unreal.Vector(-2200,-3500,4700)
rotation=unreal.MathLibrary.find_look_at_rotation(position,unreal.Vector(2600,1200,250))
unreal.EditorLevelLibrary.set_level_viewport_camera_info(position,rotation)
start=time.monotonic(); state={'shot':False}
(ROOT/'Saved/Screenshots').mkdir(exist_ok=True)
def tick(dt):
    now=time.monotonic()-start
    if now>15 and not state['shot']:
        state['shot']=True
        unreal.AutomationLibrary.take_high_res_screenshot(1600,1000,str(ROOT/'Saved/Screenshots/ObstacleCourseOverviewVerified.png'))
    elif now>23:
        (ROOT/'Saved/ObstacleCoursePreview.json').write_text(json.dumps({'success':True,'saved_actor_count':len(course),'collision_profiles':'BlockAll'}))
        unreal.unregister_slate_post_tick_callback(handle); unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
