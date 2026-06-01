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

The first milestone creates a minimal OpenGL application:

- GLFW window and input handling
- GLAD OpenGL function loading
- GLM-based `model`, `view`, and `projection` matrices
- a rotating colored cube
- optional Dear ImGui debug panel

Upcoming milestones:

1. Add a floor grid and XYZ axis.
2. Add a movable character placeholder.
3. Add planar shadows similar to the original DirectX sample.
4. Add glTF model loading.
5. Add lighting and material controls.
6. Expand documentation as a beginner-friendly 3D graphics guide.

## Tech Stack

- C++20
- CMake
- OpenGL 3.3 Core Profile
- GLFW
- GLAD, generated into `external/glad`
- GLM
- Dear ImGui
- glTF model assets, loader to be selected in a later milestone

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

## Windows / MSYS2 Setup

Use the **UCRT64** shell.

```bash
pacman -Syu
pacman -S \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-glfw \
  mingw-w64-ucrt-x86_64-glm \
  mingw-w64-ucrt-x86_64-imgui
```

Configure and build:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Run:

```bash
./build/matrixalchemy.exe
```

## Controls

- `Left` / `Right`: orbit camera horizontally
- `Up` / `Down`: orbit camera vertically
- `F1`: toggle the debug panel
- `Esc`: quit

## Development Notes

The original DirectX sample used concepts like drawable/movable objects, a
camera, a floor plane, XYZ axes, random cubes, a character model, and projected
shadows. Matrix Alchemy intentionally keeps those ideas visible in the code so
that the rendering pipeline can be learned by reading and changing the source.
