"""Render saved flight-yard geometry from two editor cameras, without PIE or saving the map."""
import unreal,time,pathlib,json
ROOT=pathlib.Path(__file__).resolve().parents[1]
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
world=unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
manifest=ROOT/'Saved/FlightYardSetup.json'
if not manifest.exists():manifest=ROOT/'QualityReports/2026-10-31-FlightYard-Setup.json'
setup=json.loads(manifest.read_text())
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
assert sum(a.get_actor_label().startswith('FlightYard_') for a in actors)==setup['new_actor_count']
def camera(pos,target):
 p=unreal.Vector(*pos)
 unreal.EditorLevelLibrary.set_level_viewport_camera_info(p,unreal.MathLibrary.find_look_at_rotation(p,unreal.Vector(*target)))
camera((700,-5700,2300),(1500,-2400,550))
shots=ROOT/'Saved/Screenshots';shots.mkdir(exist_ok=True)
start=time.monotonic();state={'stage':0}
def tick(dt):
 elapsed=time.monotonic()-start
 if elapsed>15 and state['stage']==0:
  unreal.AutomationLibrary.take_high_res_screenshot(1600,1000,str(shots/'FlightYardOverview.png'))
  state['stage']=1
 elif elapsed>21 and state['stage']==1:
  camera((1800,-3000,2600),(3200,-2225,450));state['stage']=2
 elif elapsed>27 and state['stage']==2:
  unreal.AutomationLibrary.take_high_res_screenshot(1600,1000,str(shots/'FlightHouseStairs.png'));state['stage']=3
 elif elapsed>34:
  unreal.unregister_slate_post_tick_callback(handle);unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
