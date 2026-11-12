"""Read saved materials, compile a disposable consumer and two-layer extension in memory.
No assets/maps are saved. DX12 + AllowCommandletRendering required for shader evidence.
"""
import unreal,json,os
L=unreal.MaterialEditingLibrary; E=unreal.EditorAssetLibrary; T=unreal.AssetToolsHelpers.get_asset_tools()
root='/Game/DroneTest/Materials'
master=unreal.load_asset(root+'/Masters/MM_Backrooms'); layer=unreal.load_asset(root+'/Layers/ML_Surface'); blend=unreal.load_asset(root+'/Layers/MLB_SurfaceBlend')
assert master and layer and blend
reg=unreal.AssetRegistryHelpers.get_asset_registry();reg.search_all_assets(True)
foundation=[root+'/Masters/MM_Backrooms',root+'/Layers/ML_Surface',root+'/Layers/MLB_SurfaceBlend']
assets=[reg.get_asset_by_object_path(p+'.'+p.rsplit('/',1)[-1]) for p in foundation]
assert all(a.is_valid() for a in assets)
seen=set();pending=[str(a.package_name) for a in assets];deps={}
opts=unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)
while pending:
 p=pending.pop()
 if p in seen:continue
 seen.add(p); ds=[str(x) for x in reg.get_dependencies(p,opts)];deps[p]=ds
 for d in ds:
  if d.startswith('/Game/'):
   assert d.startswith(root+'/'),('Foreign project dependency',p,d)
   assert E.does_asset_exist(d),('Missing dependency',d)
   pending.append(d)
layer_nodes=L.get_material_function_expressions(layer)
flat=next(e for e in layer_nodes if isinstance(e,unreal.MaterialExpressionScalarParameter) and str(e.get_editor_property('parameter_name'))=='Normal_Flatness')
assert flat.get_editor_property('default_value')==0
sat=next(e for e in layer_nodes if isinstance(e,unreal.MaterialExpressionSaturate))
assert flat in L.get_inputs_for_material_function_expression(layer,sat)
call=next(e for e in layer_nodes if isinstance(e,unreal.MaterialExpressionMaterialFunctionCall) and 'FlattenNormal' in str(e.get_editor_property('material_function')))
assert sat in L.get_inputs_for_material_function_expression(layer,call)
for e in layer_nodes+L.get_material_function_expressions(blend):
 if isinstance(e,unreal.MaterialExpressionTextureSampleParameter2D):
  assert e.get_editor_property('texture').get_path_name().startswith('/Engine/')
blend_names=[str(e.get_editor_property('parameter_name')) for e in L.get_material_function_expressions(blend) if isinstance(e,(unreal.MaterialExpressionScalarParameter,unreal.MaterialExpressionTextureSampleParameter2D))]
assert 'Blend_Weight' in blend_names and 'BlendMask_Texture' in blend_names
assert '1' not in blend_names and 'Param' not in blend_names
saved_node=L.get_material_expressions(master)[0]
saved_stack=saved_node.get_editor_property('default_layers')
assert len(saved_stack.get_editor_property('layers'))==1
assert len(saved_stack.get_editor_property('blends'))==0
meta=saved_stack.get_editor_property('editor_only')
for field in ['layer_states','layer_names','layer_guids','layer_link_states']:
 assert len(meta.get_editor_property(field))==1,field
assert L.get_material_property_input_node(master,unreal.MaterialProperty.MP_MATERIAL_ATTRIBUTES)==saved_node
errors=list(L.recompile_material(master));assert not errors,errors
# UE 5.8 setters return false unconditionally (MaterialEditingLibrary.cpp); verify readback instead.
# Disposable instance: no SaveAsset calls. Create/override/reset texture parameters.
consumer=T.create_asset('MI_SurfaceVerification','/Game/__MaterialVerification',unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
L.set_material_instance_parent(consumer,master)
assoc=unreal.MaterialParameterAssociation.LAYER_PARAMETER
L.set_material_instance_scalar_parameter_value(consumer,'Tiling',2.0,assoc)
L.set_material_instance_vector_parameter_value(consumer,'Base_Tint',unreal.LinearColor(0.15,0.5,0.8,1),assoc)
normal=unreal.load_asset('/Engine/EngineMaterials/DefaultNormal')
L.set_material_instance_texture_parameter_value(consumer,'Normal_Texture',normal,assoc)
L.update_material_instance(consumer)
assert abs(L.get_material_instance_scalar_parameter_value(consumer,'Tiling',assoc)-2)<0.001
assert L.get_material_instance_texture_parameter_value(consumer,'Normal_Texture',assoc)==normal
# Recovery: clearing overrides returns parent neutral textures.
L.clear_all_material_instance_parameters(consumer); L.update_material_instance(consumer)
assert abs(L.get_material_instance_scalar_parameter_value(consumer,'Tiling',assoc)-1)<0.001
assert L.get_material_instance_texture_parameter_value(consumer,'Normal_Texture',assoc)==normal
# Extension: compile two base layers plus imported blend in a disposable master.
expanded=T.duplicate_asset('M_TwoLayerVerification','/Game/__MaterialVerification',master)
node=L.get_material_expressions(expanded)[0]
# Build detached data: a live struct view notifies/recompiles on each field change.
stack=unreal.MaterialLayersFunctions()
stack.set_editor_property('layers',[layer,layer]);stack.set_editor_property('blends',[blend])
editor=stack.get_editor_property('editor_only')
editor.set_editor_property('layer_states',[True,True]);editor.set_editor_property('layer_names',['Base','Overlay'])
editor.set_editor_property('restrict_to_layer_relatives',[False,False]);editor.set_editor_property('restrict_to_blend_relatives',[False])
editor.set_editor_property('layer_guids',[unreal.GuidLibrary.new_guid(),unreal.GuidLibrary.new_guid()])
editor.set_editor_property('layer_link_states',[unreal.MaterialLayerLinkState.NOT_FROM_PARENT]*2)
stack.set_editor_property('editor_only',editor);node.set_editor_property('default_layers',stack)
extension_errors=list(L.recompile_material(expanded));assert not extension_errors,extension_errors
result={'success':True,'asset_count':len(assets),'dependencies':deps,'master_compile_errors':errors,'extension_compile_errors':extension_errors,'consumer_override_readback':True,'clear_override_recovery':True,'flatness_saturate_connected':True,'master_stats':str(L.get_statistics(master)),'extension_stats':str(L.get_statistics(expanded)),'map_modified':False,'consumer_assets_saved':False}
os.makedirs(unreal.Paths.project_saved_dir()+'MaterialImport',exist_ok=True)
with open(unreal.Paths.project_saved_dir()+'MaterialImport/Verification.json','w') as f:json.dump(result,f,indent=2)
unreal.log('SURFACE_MATERIAL_VERIFIED '+json.dumps(result))




