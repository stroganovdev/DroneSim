import unreal, json
out = {}
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
def props(obj, names):
    result = {}
    for name in names:
        try: result[name] = str(obj.get_editor_property(name))
        except Exception as e: result[name] = str(e)
    return result
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
out['world'] = props(world, ['world_type'])
ws = world.get_world_settings()
out['settings'] = props(ws, ['default_game_mode','world_partition','enable_world_bounds_checks'])
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
out['actors'] = [{'label': a.get_actor_label(), 'class': a.get_class().get_path_name(), 'location':str(a.get_actor_location())} for a in actors]
for name in ['BP_SampleDronePawn','BP_SampleDronePlayerController','BP_SampleDroneGamemode']:
    path = '/DroneSimulator/Blueprints/' + name
    cls = unreal.load_class(None, path + '.' + name + '_C')
    obj = unreal.get_default_object(cls)
    out[name] = {'class':cls.get_path_name(), 'properties':props(obj,['default_pawn_class','player_controller_class','auto_possess_player','movement_component','root_component'])}
    if name == 'BP_SampleDronePawn':
        movement = obj.get_editor_property('movement_component')
        out[name]['movement'] = props(movement,['frame_asset','motor_asset','propeller_asset','battery_asset','propulsion_model','flight_modes','active_flight_mode','updated_component'])
        out[name]['components'] = [{'name':c.get_name(),'class':c.get_class().get_name()} for c in obj.get_components_by_class(unreal.ActorComponent)]
pawn_cls = unreal.load_class(None, '/DroneSimulator/Blueprints/BP_SampleDronePawn.BP_SampleDronePawn_C')
pawn = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(pawn_cls, unreal.Vector(0,0,300))
out['spawned_pawn'] = {'root': str(pawn.get_editor_property('root_component')), 'components': []}
for c in pawn.get_components_by_class(unreal.ActorComponent):
    out['spawned_pawn']['components'].append({'name':c.get_name(), 'class':c.get_class().get_name(), 'props':props(c,['static_mesh','simulate_physics','enable_gravity','collision_profile_name','updated_component','relative_location','relative_rotation'])})
imc = unreal.load_asset('/DroneSimulator/Blueprints/IMC_MappingContext')
out['mappings'] = [str(m) for m in imc.get_editor_property('mappings')]
out['pawn_attributes'] = [n for n in dir(pawn) if not n.startswith('_')]
unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(pawn)
with open(r'E:/UnrealProjects/DroneGame/DroneSimTest/Saved/DroneTestInspection.json','w',encoding='utf-8') as f: json.dump(out,f,ensure_ascii=False,indent=2)
unreal.log('DRONE_INSPECTION_COMPLETE')


