import subprocess
import os

Import("env")

def get_firmware_specifier_build_flag():
    # GitHub Actions supplies this explicitly because checkout is shallow and
    # may not contain the tags needed by ``git describe``.
    build_version = os.environ.get("BUILD_VERSION", "").strip()
    if not build_version:
        ret = subprocess.run(
            ["git", "describe", "--tags", "--always"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
        )
        build_version = ret.stdout.strip()
    # fix unwanted and verbose tags
    build_version = build_version.removeprefix("v").replace('Release', '')
    build_flag = "-D AUTO_VERSION=\\\"" + build_version + "\\\""
    print ("Firmware Revision: " + build_version)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)
