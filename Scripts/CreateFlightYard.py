"""Build a bounded flight house/tower extension on L_DroneTest. Refuse duplicates; preserve existing actors."""
import unreal, pathlib, math, json, shutil, datetime
ROOT=pathlib.Path(__file__).resolve().parents[1]
PREFIX='FlightYard_'
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
world=unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
editor=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before=list(editor.get_all_level_actors())
assert not any(a.get_actor_label().startswith(PREFIX) for a in before),'Flight yard already exists; refusing overwrite'
def pose(a):
 p=a.get_actor_location();r=a.get_actor_rotation();s=a.get_actor_scale3d()
 return [p.x,p.y,p.z,r.pitch,r.yaw,r.roll,s.x,s.y,s.z]
old={a.get_path_name():pose(a) for a in before}
mode=world.get_world_settings().get_editor_property('default_game_mode')
backup=ROOT/'Backup'/('FlightYard_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'))
for rel in ['Content/L_DroneTest.umap','Content/__ExternalActors__/L_DroneTest','Content/__ExternalObjects__/L_DroneTest']:
 src=ROOT/rel;dst=backup/rel;dst.parent.mkdir(parents=True,exist_ok=True)
 if src.is_dir():shutil.copytree(src,dst)
 elif src.exists():shutil.copy2(src,dst)
cube=unreal.load_asset('/Engine/BasicShapes/Cube');assert cube
mats={k:unreal.load_asset('/Game/DroneTest/Obstacles/M_Course'+k) for k in ['Blue','Orange','Light']}
assert all(mats.values())
parts=[];manifest=[];names=set()
def box(name,center,size,color='Light',rotation=None):
 assert name not in names;names.add(name)
 assert all(v>0 for v in size)
 a=editor.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*center))
 assert a,name
 a.set_actor_label(PREFIX+name);a.set_folder_path('DroneTest/FlightYard/'+name.split('_')[0])
 a.set_editor_property('tags',['DroneFlightYard'])
 a.set_editor_property('is_spatially_loaded',False)
 c=a.get_component_by_class(unreal.StaticMeshComponent)
 c.set_static_mesh(cube);c.set_material(0,mats[color]);c.set_collision_profile_name('BlockAll')
 c.set_simulate_physics(False)
 a.set_actor_scale3d(unreal.Vector(*(v/100 for v in size)))
 if rotation:a.set_actor_rotation(rotation,False)
 c.set_mobility(unreal.ComponentMobility.STATIC)
 parts.append(a);manifest.append({'name':a.get_actor_label(),'center':center,'size':size,'color':color})
 return a
def beam(name,start,end,thickness=25,color='Blue'):
 start=unreal.Vector(*start);end=unreal.Vector(*end)
 mid=(start+end)*.5
 rot=unreal.MathLibrary.find_look_at_rotation(start,end)
 return box(name,[mid.x,mid.y,mid.z],[start.distance(end),thickness,thickness],color,rot)
def plate(name,x0,x1,y0,y1,top,thickness=30,color='Light',hole=None):
 if hole:
  hx0,hx1,hy0,hy1=hole
  rects=[(x0,hx0,y0,y1),(hx1,x1,y0,y1),(hx0,hx1,y0,hy0),(hx0,hx1,hy1,y1)]
 else:rects=[(x0,x1,y0,y1)]
 for i,(a,b,c,d) in enumerate(rects):
  if b>a and d>c:box(name+str(i),[(a+b)/2,(c+d)/2,top-thickness/2],[b-a,d-c,thickness],color)
def wall(name,axis,fixed,lo,hi,z0,z1,holes):
 cuts=sorted(set([lo,hi]+[v for h in holes for v in h[:2]]))
 heights=sorted(set([z0,z1]+[v for h in holes for v in h[2:]]))
 n=0
 for a,b in zip(cuts,cuts[1:]):
  for c,d in zip(heights,heights[1:]):
   mid=(a+b)/2;z=(c+d)/2
   if any(l<mid<r and bot<z<top for l,r,bot,top in holes):continue
   if axis=='X':center=[mid,fixed,z];size=[b-a,20,d-c]
   else:center=[fixed,mid,z];size=[20,b-a,d-c]
   box(name+str(n),center,size);n+=1
def window_frame(name,axis,fixed,lo,hi,z0,z1,color):
 # Thin trim stays outside the opening.
 for i,pos in enumerate([lo-8,hi+8]):
  center=[pos,fixed,(z0+z1)/2] if axis=='X' else [fixed,pos,(z0+z1)/2]
  size=[16,30,z1-z0+32] if axis=='X' else [30,16,z1-z0+32]
  box(name+'Jamb'+str(i),center,size,color)
 for i,z in enumerate([z0-8,z1+8]):
  center=[(lo+hi)/2,fixed,z] if axis=='X' else [fixed,(lo+hi)/2,z]
  size=[hi-lo,30,16] if axis=='X' else [30,hi-lo,16]
  box(name+'Lintel'+str(i),center,size,color)

# 20m x 16m practice house: ground floor 130, first floor 580, open stairwell/skylight.
x0,x1,y0,y1=2200,4200,-3400,-1800
hole=[2450,3950,-2500,-1950]
plate('House_Ground',x0,x1,y0,y1,130)
plate('House_Upper',x0,x1,y0,y1,580,hole=hole)
plate('House_Roof',x0-40,x1+40,y0-40,y1+40,1030,color='Blue',hole=hole)
for floor,z0,z1 in [('Lower',130,550),('Upper',580,1000)]:
 west=[(-3120,-2880,130,450)] if floor=='Lower' else [(-2475,-1975,650,900)]
 east=[(-3250,-2750,200,470)] if floor=='Lower' else [(-3250,-2750,650,900)]
 south=[(2400,2800,200,470),(3500,3900,200,470)] if floor=='Lower' else [(2400,2800,650,900),(3500,3900,650,900)]
 north=[(2850,3350,200,470)] if floor=='Lower' else [(2850,3350,650,900)]
 wall('House_West'+floor,'Y',x0,y0,y1,z0,z1,west)
 wall('House_East'+floor,'Y',x1,y0,y1,z0,z1,east)
 wall('House_South'+floor,'X',y0,x0,x1,z0,z1,south)
 wall('House_North'+floor,'X',y1,x0,x1,z0,z1,north)
window_frame('House_IN_','Y',x0-5,-2475,-1975,650,900,'Orange')
window_frame('House_OUT_','Y',x1+5,-3250,-2750,200,470,'Blue')
for i,(x,y) in enumerate([(x0,y0),(x0,y1),(x1,y0),(x1,y1)]):
 box('House_Corner'+str(i),[x,y,565],[40,40,870],'Blue')
plate('House_StairLanding',3750,4200,-2425,-2025,580)
# Open risers, wide treads, side supports and rails: also usable by the third-person character.
for i in range(1,24):
 top=130+i*450/23
 box('Stairs_Tread%02d'%i,[2600+(i-.5)*50,-2225,top-10],[50,400,20],'Light')
for i,y in enumerate([-2445,-2005]):
 beam('Stairs_Stringer'+str(i),(2590,y,115),(3770,y,565),20,'Blue')
 beam('Stairs_Handrail'+str(i),(2600,y,240),(3750,y,690),14,'Orange')
 for j,x in enumerate([2600,2900,3200,3500,3750]):
  z=130+(x-2600)*450/1150
  box('Stairs_RailPost%d_%d'%(i,j),[x,y,z+50],[12,12,100],'Blue')
for i,y in enumerate([-2515,-1935]):
 box('House_UpperGuard'+str(i),[3200,y,690],[1500,14,14],'Orange')
 for j,x in enumerate([2470,2950,3450,3930]):
  box('House_UpperGuardPost%d_%d'%(i,j),[x,y,635],[12,12,110],'Blue')

# A small open scaffold tower: two ring platforms, 3.6m vertical well and open west/front entrances.
tx,ty=-1100,-2400
for i,(dx,dy) in enumerate([(-300,-300),(-300,300),(300,-300),(300,300)]):
 box('Tower_Post'+str(i),[tx+dx,ty+dy,775],[35,35,1350],'Blue')
for k,top in enumerate([650,1150]):
 plate('Tower_Deck%d_'%k,tx-325,tx+325,ty-325,ty+325,top,30,'Light',[tx-180,tx+180,ty-180,ty+180])
 for side,y in enumerate([ty-310,ty+310]):
  box('Tower_Ring%d_%d'%(k,side),[tx,y,top+110],[650,18,18],'Orange')
for i,(z0,z1) in enumerate([(100,650),(650,1150),(1150,1450)]):
 beam('Tower_BackBrace'+str(i),(tx-300,ty+300,z0),(tx+300,ty+300,z1),22,'Blue')
for i,x in enumerate([tx-90,tx+90]):
 box('Tower_LadderRail'+str(i),[x,ty-360,775],[15,15,1350],'Orange')
for i in range(22):
 box('Tower_LadderRung%02d'%i,[tx,ty-360,130+i*60],[180,16,16],'Light')

# Three polygon rings with rising/falling centres, followed by the house route.
for ring,(x,z) in enumerate([(800,450),(1300,650),(1800,450)],1):
 for i in range(12):
  a=i*2*math.pi/12;b=(i+1)*2*math.pi/12
  beam('Ring%d_Segment%02d'%(ring,i),(x,-1200+240*math.cos(a),z+240*math.sin(a)),
       (x,-1200+240*math.cos(b),z+240*math.sin(b)),28,'Orange' if ring==2 else 'Blue')
 box('Ring%d_Support'%ring,[x,-1200,(100+z-240)/2],[30,30,z-240-100],'Light')

routes={
 'house_window_stairs_window':[[1900,-2225,800],[3900,-2225,800],[3725,-2225,700],
  [3225,-2225,130+13*450/23+120],[2625,-2225,130+450/23+120],[2400,-2225,250],[2400,-3000,330],[4500,-3000,330]],
 'tower_side_to_vertical':[[-1800,-2400,900],[-1100,-2400,900],[-1100,-2400,1600]],
 'rings':[[400,-1200,450],[800,-1200,450],[1300,-1200,650],[1800,-1200,450],[2100,-1200,450]]
}
# Conservative sphere-clearance check against every newly-built oriented box.
def blocked(point,radius=45):
 p=unreal.Vector(*point)
 for actor in parts:
  local=unreal.MathLibrary.inverse_transform_location(actor.get_actor_transform(),p)
  scale=actor.get_actor_scale3d()
  if all(abs(v)<50+radius/abs(s) for v,s in zip([local.x,local.y,local.z],[scale.x,scale.y,scale.z])):
   return actor.get_actor_label()
 return None
for route,points in routes.items():
 for a,b in zip(points,points[1:]):
  distance=math.dist(a,b);steps=max(1,math.ceil(distance/10))
  for i in range(steps+1):
   point=[a[j]+(b[j]-a[j])*i/steps for j in range(3)]
   hit=blocked(point)
   assert not hit,(route,point,hit)
assert all(pose(a)==old[a.get_path_name()] for a in before),'Existing actor moved'
assert world.get_world_settings().get_editor_property('default_game_mode')==mode
for actor in parts:
 c=actor.get_component_by_class(unreal.StaticMeshComponent)
 assert str(c.get_collision_profile_name())=='BlockAll' and not c.is_simulating_physics()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/L_DroneTest')
assert unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
report={'success':True,'backup':str(backup),'new_actor_count':len(parts),'existing_actor_count':len(before),
 'existing_poses_preserved':True,'game_mode_preserved':True,'clearance_radius_cm':45,'routes':routes,'parts':manifest,
 'house_bounds_cm':[x0,x1,y0,y1,100,1030],'stair_step_height_cm':450/23,'stair_tread_cm':50}
(ROOT/'Saved/FlightYardSetup.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('FLIGHT_YARD_SUCCESS actors='+str(len(parts)))
