import unreal,json,pathlib
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
bp=unreal.load_asset('/Game/DroneTest/BP_DroneTestPawn')
sub=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
rows=[]
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    data=unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h)
    obj=unreal.SubobjectDataBlueprintFunctionLibrary.get_object(data)
    row={'name':obj.get_name(),'class':obj.get_class().get_name()}
    if isinstance(obj,unreal.SceneComponent):
        row['scale']=str(obj.get_editor_property('relative_scale3d'))
    if isinstance(obj,unreal.StaticMeshComponent):
        mesh=obj.get_editor_property('static_mesh')
        row['mesh']=str(mesh)
        if mesh: row['bounds']=str(mesh.get_bounds())
    rows.append(row)
pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest/Saved/MeteorPhysicsInspection.json').write_text(json.dumps(rows,indent=2))
unreal.log('METEOR_PHYSICS_INSPECTED')
