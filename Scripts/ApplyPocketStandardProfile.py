"""Install the standard Pocket AETR/Mode 2 profile without a calibration wizard.
Uses the observed device UID and HID logical range. Preserves other devices and backups.
"""
import json, pathlib, datetime, shutil
ROOT=pathlib.Path(__file__).resolve().parents[1]
path=ROOT/'Saved/Config/DroneInputMappings.json'
original=path.read_bytes()
root=json.loads(original.decode('utf-8-sig'))
profiles=root.get('profiles',[])
pocket=[p for p in profiles if 'vid_1209&pid_4f54' in p.get('device_uid','').lower()]
assert len(pocket)==1, 'Expected exactly one previously detected Pocket profile'
uid=pocket[0]['device_uid']
caps=json.loads((ROOT/'Saved/PocketHidInspection.json').read_text(encoding='utf-8-sig'))
device=next(d for d in caps if d['device_uid'].lower()==uid.lower())
axes={a['axis']:a for a in device['axes']}
# Enum: 1=Throttle, 2=Yaw, 3=Pitch, 4=Roll. Classic CH1..4 = X,Y,Z,Rx.
layout=[('HID_0001_0030',4),('HID_0001_0031',3),('HID_0001_0032',1),('HID_0001_0033',2)]
for axis,_ in layout:
    assert axes[axis]['logical_min']==0 and axes[axis]['logical_max']==2047
profile={'device_uid':uid,
    'mappings':[{'axis':a,'game_axis':g,'is_inverted':False} for a,g in layout],
    'calibration':[{'axis':a,'min':-1.0,'max':1.0,'center':-1.0 if g==1 else 0.0} for a,g in layout]}
for i,p in enumerate(profiles):
    if p.get('device_uid')==uid: profiles[i]=profile
root['profiles']=profiles
backup=ROOT/'Backup'/('PocketProfile_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f'))
backup.mkdir(parents=True)
shutil.copy2(path,backup/path.name)
# Do not overwrite a profile that changed since this script started.
assert path.read_bytes()==original, 'Input profile changed during setup; retry after closing calibration'
path.write_text(json.dumps(root,indent=2)+'\n',encoding='utf-8')
default=ROOT/'DroneInputMappings.json'
if default.exists():
    shutil.copy2(default,backup/'DroneInputMappings.default.json')
    defaults=json.loads(default.read_text(encoding='utf-8-sig'))
else: defaults={'profiles':[]}
defaults['profiles']=[p for p in defaults.get('profiles',[]) if p.get('device_uid','').lower()!=uid.lower()]+[profile]
default.write_text(json.dumps(defaults,indent=2)+'\n',encoding='utf-8')
# Check serialized data, full spans, neutral positions and unsigned throttle endpoints.
written=json.loads(path.read_text())
p=next(p for p in written['profiles'] if p['device_uid']==uid)
assert p==profile
assert len({m['game_axis'] for m in p['mappings']})==4
for c in p['calibration']:
    assert c['max']-c['min']==2.0
    assert c['min']<=c['center']<=c['max']
throttle=next(c for c in p['calibration'] if c['axis']=='HID_0001_0032')
assert (throttle['min']-throttle['center'])/(throttle['max']-throttle['center'])==0
assert (throttle['max']-throttle['center'])/(throttle['max']-throttle['center'])==1
print('Installed standard Pocket Mode 2 / AETR. Profile checks: PASS.')
print('Backup:',backup)
print('Restart PIE (or the editor) to load the new profile. Do not run calibration again.')
