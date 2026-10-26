"""Apply the official Meteor75 Pro default Actual rates to the test Pawn only."""
import unreal, json, pathlib
root=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
bp=unreal.load_asset('/Game/DroneTest/BP_DroneTestPawn'); assert bp
cls=unreal.load_class(None,'/Game/DroneTest/BP_DroneTestPawn.BP_DroneTestPawn_C')
cdo=unreal.get_default_object(cls); movement=cdo.get_editor_property('movement_component')
modes={str(k):v for k,v in movement.get_editor_property('flight_modes').items()}; air=modes['race']
assert air.get_class().get_name()=='FlightModeAir'
report={'before':{'active_mode':str(movement.get_editor_property('active_flight_mode')),
    'rates':[air.get_editor_property(n) for n in ['roll_rate_deg_per_second','pitch_rate_deg_per_second','yaw_rate_deg_per_second']]}}
bp.modify(); movement.modify(); air.modify()
air.set_editor_property('use_actual_rates',True)
air.set_editor_property('actual_center_sensitivity',unreal.Vector(70,70,70))
air.set_editor_property('actual_expo',unreal.Vector(0,0,0))
for n in ['roll_rate_deg_per_second','pitch_rate_deg_per_second','yaw_rate_deg_per_second']: air.set_editor_property(n,670.0)
movement.set_editor_property('active_flight_mode','race')
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
report['after']={'active_mode':'race','center_sensitivity_deg_s':70,'maximum_deg_s':670,'expo':0,'angle_retained':'angle' in modes,
    'source_cli':'https://support.betafpv.com/hc/article_attachments/55526741863833',
    'source_defaults':'https://github.com/betaflight/betaflight/blob/4.5.3/src/main/fc/controlrate_profile.c',
    'curve':'https://github.com/betaflight/betaflight/blob/4.5.3/src/main/fc/rc.c'}
(root/'Saved/Meteor75Rates.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('METEOR75_RATES_APPLIED '+json.dumps(report))
