#!/usr/bin/env python3
"""Package PlatformIO builds for the DIY Flasher.

Each PlatformIO environment is exported to ``<version>_<environment>``.  The
resulting ``index.json`` can be served alongside the files and consumed by the
browser flasher at https://valerio-vaccaro.github.io/diyflasher/.
"""

import argparse
import hashlib
import json
import shutil
from datetime import datetime, timezone
from pathlib import Path


ESP32_FLASH_FILES = (
    (0x1000, "bootloader.bin"),
    (0x8000, "partitions.bin"),
    (0xE000, "boot_app0.bin"),
    (0x10000, "firmware.bin"),
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def flash_files_for(environment: Path) -> tuple[tuple[int, str], ...]:
    """Return the flash layout based on the files produced by PlatformIO."""
    if all((environment / name).exists() for _, name in ESP32_FLASH_FILES):
        return ESP32_FLASH_FILES

    firmware = environment / "firmware.bin"
    if firmware.exists():
        companion_images = [
            environment / name for _, name in ESP32_FLASH_FILES[:-1]
        ]
        if any(image.exists() for image in companion_images):
            raise FileNotFoundError(
                f"{environment} has an incomplete ESP32 flash layout"
            )
        # ESP8266 PlatformIO builds are a complete image flashed at address 0.
        return ((0x0000, "firmware.bin"),)

    expected = ", ".join(name for _, name in ESP32_FLASH_FILES)
    raise FileNotFoundError(
        f"{environment} does not contain a supported firmware layout; expected {expected}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, default=Path(".pio/build"))
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--name", default="BTC-PB")
    parser.add_argument("--base-url", default=".")
    args = parser.parse_args()

    if not args.version or "/" in args.version or "\\" in args.version:
        raise ValueError("--version must be a non-empty path-safe value")
    if not args.build_dir.is_dir():
        raise FileNotFoundError(f"Build directory does not exist: {args.build_dir}")

    args.output.mkdir(parents=True, exist_ok=True)
    builds = []
    environments = sorted(path for path in args.build_dir.iterdir() if path.is_dir())
    if not environments:
        raise FileNotFoundError(f"No PlatformIO environments found in {args.build_dir}")

    for environment in environments:
        board_folder = f"{args.version}_{environment.name}"
        board_output = args.output / board_folder
        board_output.mkdir(parents=True, exist_ok=True)
        files = []

        for address, source_name in flash_files_for(environment):
            source = environment / source_name
            target_name = f"0x{address:04X}_{source_name}"
            target = board_output / target_name
            shutil.copyfile(source, target)
            files.append(
                {
                    "address": f"0x{address:X}",
                    "file": f"{board_folder}/{target_name}",
                    "sha256": sha256(target),
                    "size": target.stat().st_size,
                }
            )

        builds.append(
            {
                "name": environment.name,
                "platformio_environment": environment.name,
                "files": files,
                "flashFiles": files,
            }
        )

    manifest = {
        "name": args.name,
        "version": args.version,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "base_url": args.base_url,
        "builds": builds,
        "versions": [{"version": args.version, "builds": builds}],
    }
    (args.output / "index.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
