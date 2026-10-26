import unreal, pathlib, json
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
el=unreal.EditorAssetLibrary
path='/Game/DroneTest/BP_DroneTestPlayerController'
assert not el.does_asset_exist(path), 'Refusing to overwrite an existing controller Blueprint'
bp=el.duplicate_asset('/DroneSimulator/Blueprints/BP_SampleDronePlayerController',path)
parent=unreal.load_class(None,'/Script/DroneSimTest.DroneTestPlayerController'); assert parent
unreal.BlueprintEditorLibrary.reparent_blueprint(bp,parent)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert el.save_loaded_asset(bp,False)
cls=unreal.load_class(None,path+'.BP_DroneTestPlayerController_C'); assert cls
mode=unreal.load_asset('/Game/DroneTest/BP_DroneTestGameMode')
mode_cls=unreal.load_class(None,'/Game/DroneTest/BP_DroneTestGameMode.BP_DroneTestGameMode_C')
mode.modify()
unreal.get_default_object(mode_cls).set_editor_property('player_controller_class',cls)
assert el.save_loaded_asset(mode,False)
pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest/Saved/PauseMenuSetup.json').write_text(json.dumps({'controller':cls.get_path_name(),'parent':parent.get_path_name(),'game_mode':mode_cls.get_path_name()},indent=2))
unreal.log('PAUSE_MENU_LINKED')
