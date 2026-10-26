"""Read-only HID inspection of the connected Pocket; no guessed axis mapping."""
import ctypes as C, ctypes.wintypes as W, winreg, json
base = r'SYSTEM\CurrentControlSet\Control\DeviceClasses\{4d1e55b2-f16f-11cf-88cb-001111000030}'
paths=[]
with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,base) as k:
    for i in range(winreg.QueryInfoKey(k)[0]):
        name=winreg.EnumKey(k,i)
        if 'vid_1209&pid_4f54' in name.lower():
            paths.append(chr(92)*2 + '?' + chr(92) + name[4:])
k32=C.WinDLL('kernel32',use_last_error=True); hid=C.WinDLL('hid',use_last_error=True)
k32.CreateFileW.argtypes=[W.LPCWSTR,W.DWORD,W.DWORD,C.c_void_p,W.DWORD,W.DWORD,W.HANDLE]; k32.CreateFileW.restype=W.HANDLE
k32.CloseHandle.argtypes=[W.HANDLE]
hid.HidD_GetProductString.argtypes=[W.HANDLE,C.c_void_p,W.ULONG]
hid.HidD_GetPreparsedData.argtypes=[W.HANDLE,C.POINTER(C.c_void_p)]
hid.HidD_FreePreparsedData.argtypes=[C.c_void_p]
hid.HidP_GetCaps.argtypes=[C.c_void_p,C.c_void_p]
hid.HidP_GetValueCaps.argtypes=[C.c_int,C.c_void_p,C.POINTER(W.USHORT),C.c_void_p]
class Caps(C.Structure):
    _fields_=[(n,W.USHORT) for n in ['usage','usage_page','input_bytes','output_bytes','feature_bytes']]+[('reserved',W.USHORT*17)]+[(n,W.USHORT) for n in ['links','input_buttons','input_values','input_indices','output_buttons','output_values','output_indices','feature_buttons','feature_values','feature_indices']]
results=[]
for path in paths:
    handle=k32.CreateFileW(path,0xC0000000,3,None,3,0x40000000,None)
    if handle == C.c_void_p(-1).value: raise OSError(C.get_last_error())
    pp=C.c_void_p(); caps=Caps(); name=C.create_unicode_buffer(128)
    try:
        hid.HidD_GetProductString(handle,name,C.sizeof(name))
        assert hid.HidD_GetPreparsedData(handle,C.byref(pp))
        assert hid.HidP_GetCaps(pp,C.byref(caps)) == 0x110000
        count=W.USHORT(caps.input_values); values=C.create_string_buffer(72*count.value)
        assert hid.HidP_GetValueCaps(0,values,C.byref(count),pp)==0x110000
        import struct
        axes=[]
        for i in range(count.value):
            b=values.raw[i*72:(i+1)*72]
            page=struct.unpack_from('<H',b,0)[0]; usage=struct.unpack_from('<H',b,56)[0]
            axes.append({'axis':f'HID_{page:04X}_{usage:04X}','bit_size':struct.unpack_from('<H',b,18)[0],'logical_min':struct.unpack_from('<i',b,40)[0],'logical_max':struct.unpack_from('<i',b,44)[0]})
        results.append({'product':name.value,'device_uid':path,'usage_page':caps.usage_page,'usage':caps.usage,'input_report_bytes':caps.input_bytes,'axes':axes,'read_write_open':True})
    finally:
        if pp: hid.HidD_FreePreparsedData(pp)
        k32.CloseHandle(handle)
print(json.dumps(results,indent=2))
with open(r'E:/UnrealProjects/DroneGame/DroneSimTest/Saved/PocketHidInspection.json','w') as f: json.dump(results,f,indent=2)


