"""Fail the build if OTA identity is missing, moved or inconsistent."""
Import("env")
from pathlib import Path
import re,struct,json
def check(source,target,env):
    data=Path(str(target[0])).read_bytes()
    header=(Path(env.subst("$PROJECT_DIR"))/"include/FirmwareIdentity.h").read_text()
    expected=[re.search(r'#define IOT_FIRMWARE_'+key+r' "([^"]+)"',header).group(1) for key in ("PRODUCT","HARDWARE","VERSION")]
    chip=int(re.search(r'#define IOT_FIRMWARE_CHIP (\d+)',header).group(1))
    assert len(data)>392 and data[0]==0xe9 and struct.unpack_from("<H",data,12)[0]==chip,"Wrong ESP application"
    assert data[288:296]==b"IOTFW01\0","OTA descriptor missing at fixed offset 288"
    actual=[data[i:i+32].split(b"\0",1)[0].decode("ascii") for i in (296,328,360)]
    assert actual==expected,(actual,expected)
    Path(str(target[0])+".json").write_text(json.dumps(dict(product=actual[0],hardware=actual[1],version=actual[2],size=len(data)),indent=2))
    print("Verified OTA identity:"," / ".join(actual))
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin",check)
