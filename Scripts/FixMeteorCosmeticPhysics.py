import unreal, pathlib
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
bp=unreal.load_asset('/Game/DroneTest/BP_DroneTestPawn'); bp.modify()
sub=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem); seen=set()
for h in sub.k2_gather_subobject_data_for_blueprint(bp):
    obj=unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))
    if not isinstance(obj,unreal.StaticMeshComponent) or obj.get_path_name() in seen: continue
    seen.add(obj.get_path_name()); obj.modify()
    bi=obj.get_editor_property('body_instance'); bi.set_editor_property('auto_weld',False)
    if obj.get_name()=='Cube_GEN_VARIABLE':
        bi.set_editor_property('inertia_conditioning',False)
        bi.set_editor_property('use_ccd',True)
    else:
        bi.set_editor_property('simulate_physics',False)
    obj.set_editor_property('body_instance',bi)
    if obj.get_name()!='Cube_GEN_VARIABLE': obj.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
unreal.log('METEOR_COSMETIC_WELD_DISABLED '+str(len(seen)))
