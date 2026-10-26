import unreal,time,pathlib,json,traceback
ROOT=pathlib.Path(r'E:/UnrealProjects/DroneGame/DroneSimTest')
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/L_DroneTest')
start=time.monotonic(); state={'requested':False,'shot':False,'section':0,'wall':0}
shots=['PauseMenuControls','PauseMenuAircraft','PauseMenuRadio','PauseMenuCamera']
(ROOT/'Saved/Screenshots').mkdir(exist_ok=True)
def tick(dt):
    now=time.monotonic()-start
    if not state['requested'] and now>2:
        level.editor_request_begin_play(); state['requested']=True
    if level.is_in_play_in_editor():
        world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        pc=unreal.GameplayStatics.get_player_controller(world,0)
        if not state['shot'] and now>8:
            pc.open_pause_menu(); state['shot']=True; state['wall']=now
        elif state['shot'] and now-state['wall']>2 and state['section']<4:
            menu=pc.get_pause_menu(); menu.select_section(state['section'])
            state['section']+=1; state['wall']=now; state['capture_pending']=True
        elif state.get('capture_pending') and now-state['wall']>0.5:
            name=shots[state['section']-1]
            unreal.SystemLibrary.execute_console_command(world,'Shot SHOWUI filename='+str(ROOT/'Saved/Screenshots'/name)+'.png')
            state['capture_pending']=False
        elif state['section']==4 and not state.get('capture_pending') and now-state['wall']>2:
            pc.close_pause_menu(); unreal.unregister_slate_post_tick_callback(handle)
            level.editor_request_end_play(); unreal.SystemLibrary.quit_editor()
    if now>120:
        unreal.unregister_slate_post_tick_callback(handle); unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
