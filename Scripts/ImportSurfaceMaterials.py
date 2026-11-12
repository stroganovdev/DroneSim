"""One-time minimal AnomalyLoop material import. Never overwrites destination assets.
Stage only MM_Backrooms, ML_Yellow_Wall and MLB at their original package paths first.
No map assignments; original project is read-only. Run with DX12 commandlet rendering.
"""
import unreal,json,os
L=unreal.MaterialEditingLibrary
E=unreal.EditorAssetLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
ROOT='/Game/DroneTest/Materials'
paths=[ROOT+'/Layers/ML_Surface',ROOT+'/Layers/MLB_SurfaceBlend',ROOT+'/Masters/MM_Backrooms']
assert not any(E.does_asset_exist(p) for p in paths),'Destination exists; refusing overwrite'
base='/Game/My_Game/Common/Master_Material/'
layer=T.duplicate_asset('ML_Surface',ROOT+'/Layers',unreal.load_asset(base+'ML_Yellow_Wall'))
blend=T.duplicate_asset('MLB_SurfaceBlend',ROOT+'/Layers',unreal.load_asset(base+'MLB'))
master=T.duplicate_asset('MM_Backrooms',ROOT+'/Masters',unreal.load_asset(base+'MM_Backrooms'))
assert layer and blend and master
# Engine neutral resources, no old-project texture packages.
white=unreal.load_asset('/Engine/EngineResources/WhiteSquareTexture')
normal=unreal.load_asset('/Engine/EngineMaterials/DefaultNormal')
assert white and normal
for e in L.get_material_function_expressions(layer):
 if isinstance(e,unreal.MaterialExpressionTextureSampleParameter2D):
  name=str(e.get_editor_property('parameter_name'))
  e.set_editor_property('texture',normal if name=='Normal_Texture' else white)
 if isinstance(e,unreal.MaterialExpressionScalarParameter) and str(e.get_editor_property('parameter_name'))=='Normal_Flatness':
  e.set_editor_property('default_value',0.0);e.set_editor_property('slider_min',0.0);e.set_editor_property('slider_max',1.0)
  flat=e
call=next(e for e in L.get_material_function_expressions(layer) if isinstance(e,unreal.MaterialExpressionMaterialFunctionCall) and 'FlattenNormal' in str(e.get_editor_property('material_function')))
clamp=L.create_material_expression_in_function(layer,unreal.MaterialExpressionSaturate,-300,200)
assert L.connect_material_expressions(flat,'',clamp,'')
assert L.connect_material_expressions(clamp,'',call,'Flatness')
for e in L.get_material_function_expressions(blend):
 if isinstance(e,unreal.MaterialExpressionTextureSampleParameter2D):
  e.set_editor_property('texture',white);e.set_editor_property('parameter_name','BlendMask_Texture')
 if isinstance(e,unreal.MaterialExpressionScalarParameter):
  name=str(e.get_editor_property('parameter_name'))
  if name=='1':e.set_editor_property('parameter_name','Blend_Weight')
  if name=='Mask_Intensity':e.set_editor_property('default_value',0.0)
L.update_material_function(layer);L.update_material_function(blend)
node=next(e for e in L.get_material_expressions(master) if isinstance(e,unreal.MaterialExpressionMaterialAttributeLayers))
stack=node.get_editor_property('default_layers')
unreal.log('STACK_BEFORE '+str(stack))
stack.set_editor_property('layers',[layer]);stack.set_editor_property('blends',[])
node.set_editor_property('default_layers',stack)
errors=list(L.recompile_material(master)); assert not errors,errors
for o in [layer,blend,master]:assert E.save_loaded_asset(o,False)
result={'success':True,'assets':paths,'compile_errors':errors,'normal_default':normal.get_path_name(),'color_default':white.get_path_name()}
os.makedirs(unreal.Paths.project_saved_dir()+'MaterialImport',exist_ok=True)
with open(unreal.Paths.project_saved_dir()+'MaterialImport/Import.json','w') as f:json.dump(result,f,indent=2)
unreal.log('MINIMAL_MATERIAL_IMPORT_SUCCESS')

