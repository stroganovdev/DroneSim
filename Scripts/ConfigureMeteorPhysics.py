"""Configure the analog Meteor75 Pro (1102 22000KV / 45mm / 1S) approximation."""
import unreal, pathlib, json, math
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
el=unreal.EditorAssetLibrary
folder='/Game/DroneTest/Meteor75Pro'; el.make_directory(folder)
def asset(name,cls):
    path=folder+'/'+name
    if el.does_asset_exist(path):
        assert '-ResumeMeteorPhysicsSetup' in unreal.SystemLibrary.get_command_line(), 'Refusing to overwrite '+path
        obj=unreal.load_asset(path); assert obj.get_class()==cls.static_class()
        return obj
    obj=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,cls,unreal.DataAssetFactory())
    assert obj, path
    return obj
frame=asset('Frame_Meteor75Pro_Analog',unreal.DroneFrameAsset)
# Dry weight includes motors. Frame mass here includes all remaining dry components.
frame.set_editor_property('mass_kg',0.031-4*0.00279)
arm_cm=8.08/(2*math.sqrt(2))
frame.set_editor_property('props_extent_front_cm',unreal.Vector(arm_cm,arm_cm,0))
frame.set_editor_property('props_extent_back_cm',unreal.Vector(-arm_cm,arm_cm,0))
frame.set_editor_property('area_m2',unreal.Vector(0.0012,0.0012,0.0015))
frame.set_editor_property('drag_coefficient',unreal.Vector(1.1,1.1,1.0))
frame.set_editor_property('inertia_override_kg_m2',unreal.Vector(0.000021,0.000025,0.000042))
frame.set_editor_property('rotational_drag_coefficient',unreal.Vector(0.00000015,0.00000015,0.00000010))
motor=asset('Motor_1102_22000KV',unreal.DroneMotorAsset)
motor.set_editor_property('kv',22000.0); motor.set_editor_property('mass_kg',0.00279)
battery=asset('Battery_1S_550mAh_Loaded',unreal.DroneBatteryAsset)
battery.set_editor_property('voltage',3.8); battery.set_editor_property('mass_kg',0.014)
prop=asset('Prop_GF45mm_3B_Estimated',unreal.DronePropellerSimplifiedAsset)
prop.set_editor_property('diameter_inch',45.0/25.4)
# Effective Ct fitted to BETAFPV 1102/22000KV's 46.3gf @4V, GF-1635-3B.
# Applying this Ct to 45mm is an explicitly documented extrapolation.
ct=0.0463*9.81/(1.225*(22000*4/60)**2*0.040**4)
prop.set_editor_property('thrust_coefficient',ct); prop.set_editor_property('torque_coefficient',0.005)
for obj in [frame,motor,battery,prop]: assert el.save_loaded_asset(obj,False)
bp=unreal.load_asset('/Game/DroneTest/BP_DroneTestPawn')
cls=unreal.load_class(None,'/Game/DroneTest/BP_DroneTestPawn.BP_DroneTestPawn_C')
cdo=unreal.get_default_object(cls); movement=cdo.get_editor_property('movement_component')
bp.modify(); movement.modify()
for n,o in [('frame_asset',frame),('motor_asset',motor),('battery_asset',battery),('propeller_asset',prop)]: movement.set_editor_property(n,o)
model=unreal.new_object(unreal.PropulsionModelDynamics,outer=movement,name='Meteor75Dynamics')
pid=unreal.new_object(unreal.PidDroneController,outer=model,name='Meteor75RateController')
for n,values in [('roll_pid',(0.009,0.02,0.0)),('pitch_pid',(0.0107,0.02,0.0)),('yaw_pid',(0.07,0.05,0.0))]:
    conf=pid.get_editor_property(n)
    for field,v in zip(['proportional','integral','derivative'],values): conf.set_editor_property(field,v)
    pid.set_editor_property(n,conf)
pid.set_editor_property('min_throttle',0.05); pid.set_editor_property('min_dynamic_throttle',0.02)
model.set_editor_property('drone_controller',pid)
model.set_editor_property('rotor_model',unreal.new_object(unreal.load_class(None,'/Script/DroneSimulatorCore.RotorModelSimplified'),outer=model,name='Meteor75Rotors'))
model.set_editor_property('motor_response_time_seconds',0.025)
movement.set_editor_property('propulsion_model',model)
movement.set_editor_property('active_flight_mode','race')
# Keep the already established rates and input profile. Adjust only the backup Angle mode.
modes={str(k):v for k,v in movement.get_editor_property('flight_modes').items()}
modes['angle'].modify()
modes['angle'].set_editor_property('max_roll_angle_deg',50.0)
modes['angle'].set_editor_property('max_pitch_angle_deg',50.0)
modes['angle'].set_editor_property('yaw_rate_deg_per_second',670.0)
# Scale the existing cosmetic mesh to a 110mm envelope and move its motor centers to 80.8mm diagonal.
sub=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem); components={}
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    obj=unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    components[obj.get_name()]=obj
body=components['Cube_GEN_VARIABLE']; body.modify()
scale=5.5/body.get_editor_property('static_mesh').get_bounds().box_extent.y
body.set_editor_property('relative_scale3d',unreal.Vector(scale,scale,scale))
body.set_linear_damping(0.0); body.set_angular_damping(0.0)
for suffix,x,y in [('FL',1,-1),('FR',1,1),('RL',-1,-1),('RR',-1,1)]:
    comp=components['Motor'+suffix+'_GEN_VARIABLE']; comp.modify()
    comp.set_editor_property('relative_location',unreal.Vector(x*arm_cm/scale,y*arm_cm/scale,0.6))
    pcomp=components['Prop'+suffix+'_GEN_VARIABLE']; pcomp.modify()
    ps=4.5/(2*pcomp.get_editor_property('static_mesh').get_bounds().box_extent.x*scale)
    pcomp.set_editor_property('relative_scale3d',unreal.Vector(ps,ps,ps))
# Cameras are from the existing sample, no new mesh or camera classes.
components['SpringArm_3P_GEN_VARIABLE'].set_editor_property('target_arm_length',60.0)
assert el.save_loaded_asset(bp,False)
full=ct*1.225*(22000*3.8/60)**2*0.045**4
report={'variant':'analog Meteor75 Pro, 2023-style 1102 22000KV', 'dry_mass_kg':0.031,'battery_mass_kg':0.014,'total_mass_kg':0.045,
 'wheelbase_mm':80.8,'prop_diameter_mm':45,'loaded_voltage':3.8,'motor_kv':22000,'motor_mass_kg':0.00279,
 'inertia_kg_m2':[0.000021,0.000025,0.000042],'thrust_coefficient':ct,'torque_coefficient':0.005,
 'estimated_max_thrust_per_motor_N':full,'estimated_thrust_to_weight':4*full/(0.045*9.81),
 'estimated_hover_throttle':math.sqrt(0.045*9.81/(4*full)),'motor_response_seconds':0.025,
 'estimated_acceleration_at_60pct_m_s2':4*full*0.6**2/0.045-9.81,'root_mesh_scale':scale,
 'parts':[o.get_path_name() for o in [frame,motor,battery,prop]],'propulsion_model':model.get_class().get_name()}
(ROOT/'Saved/Meteor75Physics.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('METEOR_PHYSICS_CONFIGURED '+json.dumps(report))
