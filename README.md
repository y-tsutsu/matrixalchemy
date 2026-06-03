# Matrix Alchemy

Matrix Alchemy is a small C++ OpenGL playground for learning the fundamentals of
3D graphics: transforms, camera matrices, lighting, shadows, model loading, and
debug tooling.

The project is inspired by an older Managed DirectX sample and rebuilds the same
kind of experience with a modern, portable native stack.

## Goals

- Keep the code approachable and class-based.
- Make matrix transforms visible in the implementation.
- Build and run on Debian/WSLg and Windows/MSYS2.
- Add features in small, reviewable commits.
- Prefer simple graphics techniques first, then evolve them step by step.

## Current Milestone

The current milestone provides a minimal OpenGL scene:

- GLFW window and input handling
- GLAD OpenGL function loading
- GLM-based `model`, `view`, and `projection` matrices
- GLSL shader files embedded into the executable at build time
- a checkerboard floor grid
- RGB XYZ axes
- a rotating colored cube
- a keyboard-controlled VRM character preview
- a VRM preview model loaded through cgltf with node transforms, vertex colors,
  base color factors, base color textures, alpha modes, double-sided materials,
  and texture sampler state
- mouse-driven orbit camera controls
- planar shadows projected onto the floor
- optional Dear ImGui debug panel

Upcoming milestones:

1. Improve the VRM character preview with toon-style presentation controls.
2. Add lighting and material controls.
3. Expand documentation as a beginner-friendly 3D graphics guide.

## Tech Stack

- C++20
- CMake
- OpenGL 3.3 Core Profile
- GLFW
- GLAD, generated into `external/glad`
- GLM
- Dear ImGui
- cgltf, vendored under `external/cgltf` for local builds and listed in
  `vcpkg.json` for Windows/vcpkg builds
- stb_image for PNG/JPEG texture decoding
- glTF model assets

## Debian / WSLg Setup

Install the build tools and runtime dependencies:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  python3-glad \
  libglfw3-dev \
  libglm-dev \
  libstb-dev \
  libgl-dev \
  libimgui-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev
```

The repository contains generated GLAD sources under `external/glad`. They can
be regenerated with:

```bash
glad --reproducible --api gl:core=3.3 --out-path external/glad c
```

Dear ImGui is optional at configure time. When `libimgui-dev` is available, the
debug panel is enabled automatically through `pkg-config`.

Configure and build:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Run:

```bash
./build/matrixalchemy
```

## Windows / MSYS2 + vcpkg Setup

Use MSYS2 mainly for the shell and build tools. C++ libraries are managed by
vcpkg so that the same dependencies can also be used from Visual Studio later.

In the **UCRT64** shell, install the development tools:

```bash
pacman -Syu
pacman -S \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  git
```

Install vcpkg separately, then install this project's dependencies through the
manifest:

```bash
git clone https://github.com/microsoft/vcpkg.git /c/dev/vcpkg
/c/dev/vcpkg/bootstrap-vcpkg.sh
/c/dev/vcpkg/vcpkg install --triplet x64-mingw-dynamic
```

Configure and build:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/c/dev/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic
cmake --build build
```

Run:

```bash
./build/matrixalchemy.exe
```

## Windows / Visual Studio + vcpkg Setup

Install vcpkg and use the same `vcpkg.json` manifest. A typical configure step
from PowerShell is:

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build build
```

## Controls

- `Left` / `Right`: turn the character
- `Up` / `Down`: move the character forward/backward
- Left mouse drag: orbit the camera
- Mouse wheel: zoom the camera
- `F1`: toggle the debug panel
- `Esc`: quit

## Development Notes

The original DirectX sample used concepts like drawable/movable objects, a
camera, a floor plane, XYZ axes, random cubes, a character model, and projected
shadows. Matrix Alchemy intentionally keeps those ideas visible in the code so
that the rendering pipeline can be learned by reading and changing the source.

Shader sources live under `assets/shaders` for editing. CMake converts them into
a generated C++ header under the build directory, so the executable does not need
to load shader files at runtime.

The scene uses `assets/models/saurus.vrm` as the current keyboard-controlled VRM
character. The build copies it next to the executable as `saurus.vrm`, and the
runtime searches for that file next to the executable and in the current working
directory. If the runtime model is not available, the app falls back to the
simple box character and the small `simple-triangle.gltf` sample.

The source tree is grouped by role. Headers mirror the implementation
directories under `include/matrixalchemy`:

- `app`: application lifetime, input, and render ordering
- `platform`: filesystem and platform/OpenGL include boundaries
- `render`: OpenGL rendering primitives, shaders, textures, and shadow helpers
- `asset`: glTF/VRM loading and conversion
- `scene`: scene objects, camera, and drawable/shadow-casting interfaces
- `ui`: Dear ImGui debug UI

## VRM Support Scope

The current VRM support is intentionally small and focused on rendering a
beginner-friendly character sample in native OpenGL. It supports:

- binary glTF/GLB-based `.vrm` files
- scene node transforms
- triangle meshes with indices
- `POSITION`, `NORMAL`, `TEXCOORD_0`, and `COLOR_0`
- base color factors and base color textures
- embedded and external PNG/JPEG textures
- glTF texture sampler state
- alpha mask, alpha blend, and double-sided materials
- `KHR_texture_transform`
- keyboard-driven movement through the sample `Character` transform
- projected floor shadows
- a simple normal-expanded toon-style outline

The loader does not yet implement the full VRM feature set. These are currently
out of scope:

- skeletal skinning
- animation playback
- humanoid retargeting
- blend shapes and facial expressions
- spring bones
- first-person settings
- full MToon material reproduction
- VRM 1.0 extension-specific runtime behavior

## Asset Credits

- `assets/models/saurus.vrm`: "Cute Saurus", distributed through Open Source
  Avatars. License: CC0 1.0 Universal.
  Source: <https://www.opensourceavatars.com/>
