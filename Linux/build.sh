#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
if [[ ! -f ../Gambatte/src/libgambatte/gambatte.cpp || ! -f ../mGBA/src/gba/gba.c || ! -f ../SNES9x/src/snes9x.cpp ]]; then
    git -C .. submodule update --init --depth 1 Gambatte mGBA SNES9x
fi
"${CMAKE:-cmake}" -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "$@"
"${CMAKE:-cmake}" --build build --parallel "${BUILD_JOBS:-4}"
printf '%s\n' 'Build complete. Launch with ./run.sh'
