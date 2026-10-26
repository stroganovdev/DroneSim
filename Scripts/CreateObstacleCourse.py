"""Add a bounded obstacle course to the existing map; never replace existing actors."""
import unreal, pathlib, shutil, datetime, json, math
ROOT = pathlib.Path(__file__).resolve().parents[1]
PREFIX = 'DroneCourse_'
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
existing = list(actors.get_all_level_actors())
assert not any(a.get_actor_label().startswith(PREFIX) for a in existing), 'Course already exists; refusing duplicate/overwrite'
starts = [a for a in existing if isinstance(a, unreal.PlayerStart)]
assert len(starts) == 1
def pose(a):
    p=a.get_actor_location(); r=a.get_actor_rotation(); s=a.get_actor_scale3d()
    return (p.x,p.y,p.z,r.pitch,r.yaw,r.roll,s.x,s.y,s.z)
start_before = pose(starts[0])
gamemode_before = world.get_world_settings().get_editor_property('default_game_mode')
backup = ROOT/'Backup'/('ObstacleCourse_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'))
for rel in ['Content/L_DroneTest.umap', 'Content/__ExternalActors__/L_DroneTest', 'Content/__ExternalObjects__/L_DroneTest']:
    src = ROOT/rel; dst = backup/rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    if src.is_dir(): shutil.copytree(src, dst)
    elif src.exists(): shutil.copy2(src, dst)
mesh = unreal.load_asset('/Engine/BasicShapes/Cube')
assert mesh
materials = {}
for name, color in [('Blue',(0.025,0.32,0.7)), ('Orange',(0.95,0.21,0.025)), ('Light',(0.78,0.85,0.9))]:
    path = '/Game/DroneTest/Obstacles/M_Course'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        materials[name] = unreal.load_asset(path)
        continue  # Reuse existing materials without overwriting them.
    unreal.EditorAssetLibrary.make_directory('/Game/DroneTest/Obstacles')
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Course'+name, '/Game/DroneTest/Obstacles', unreal.Material, unreal.MaterialFactoryNew())
    node = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -200, 0)
    node.set_editor_property('constant', unreal.LinearColor(*color,1))
    unreal.MaterialEditingLibrary.connect_material_property(node, '', unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
    materials[name] = mat
created=[]

def box(name, location, size, color='Blue', yaw=0):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label(PREFIX+name)
    actor.set_folder_path('DroneTest/ObstacleCourse')
    actor.set_editor_property('tags', ['DroneTestObstacleCourse'])
    actor.set_editor_property('is_spatially_loaded', False)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    component.set_material(0, materials[color])
    component.set_collision_profile_name('BlockAll')
    component.set_simulate_physics(False)
    actor.set_actor_scale3d(unreal.Vector(*(v/100 for v in size)))
    actor.set_actor_rotation(unreal.Rotator(yaw=yaw), False)
    component.set_mobility(unreal.ComponentMobility.STATIC)
    created.append(actor)
    return actor

# Positions in cm; +X is ahead of the unchanged PlayerStart.
for i,(x,y,width,height) in enumerate([(3400,-400,180,200),(4000,400,220,300),(4700,-600,250,400),(5400,2700,300,300),(1700,3200,200,250),(-800,2400,250,350)],1):
    box('SlalomCube%02d'%i,(x,y,100+height/2),(width,width,height),'Orange')

tunnels=[]
def tunnel(name,x,y,yaw,length,clear_width,clear_height,bottom=120):
    thickness=20
    def local(lx,ly,z):
        r=math.radians(yaw)
        return (x+lx*math.cos(r)-ly*math.sin(r),y+lx*math.sin(r)+ly*math.cos(r),z)
    for side,sy in [('Left',-1),('Right',1)]:
        box(name+side,local(0,sy*(clear_width/2+thickness/2),bottom+clear_height/2),(length,thickness,clear_height),'Blue',yaw)
    box(name+'Roof',local(0,0,bottom+clear_height+thickness/2),(length,clear_width+2*thickness,thickness),'Blue',yaw)
    box(name+'Floor',local(0,0,bottom-thickness/2),(length,clear_width+2*thickness,thickness),'Light',yaw)
    # Thin orange lintels mark both entrances without reducing the clear opening.
    for side,sx in [('Entry',-1),('Exit',1)]:
        box(name+side,local(sx*(length/2-10),0,bottom+clear_height+thickness/2),(20,clear_width+2*thickness,thickness),'Orange',yaw)
    tunnels.append({'name':name,'center':[x,y,bottom+clear_height/2],'yaw':yaw,'length_cm':length,'clear_width_cm':clear_width,'clear_height_cm':clear_height})

tunnel('Tunnel01_',2200,0,0,600,360,320)
tunnel('Tunnel02_',5200,1300,90,800,300,300)
tunnel('Tunnel03_',3000,2900,0,900,260,260)
tunnel('Tunnel04_',0,2100,90,500,360,360,400)
# Supports for the raised final tunnel, outside its flight corridor.
for x in [-200,200]:
    for y in [1900,2300]:
        box('RaisedSupport_%d_%d'%(x,y),(x,y,240),(30,30,280),'Light')

assert len(created)==34
assert pose(starts[0]) == start_before
assert world.get_world_settings().get_editor_property('default_game_mode') == gamemode_before
for actor in created:
    component=actor.get_component_by_class(unreal.StaticMeshComponent)
    assert component.get_editor_property('static_mesh') == mesh
    assert str(component.get_collision_profile_name()) == 'BlockAll'
    assert not component.is_simulating_physics()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/L_DroneTest')
assert unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
report={'success':True,'backup':str(backup),'actor_count':len(created),'cubes':6,'tunnels':tunnels,'start_unchanged':True,'gamemode_unchanged':True,'actors':[a.get_actor_label() for a in created]}
(ROOT/'Saved').mkdir(exist_ok=True)
(ROOT/'Saved/ObstacleCourseSetup.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
unreal.log('OBSTACLE_COURSE_SUCCESS '+json.dumps(report))
