# Graphics Notes

This document is a reading guide for Matrix Alchemy's current rendering sample.
It focuses on the graphics concepts that are visible in the implementation and
points to the source files where each idea appears.

## Coordinate System and Matrices

Matrix Alchemy uses OpenGL-style 3D rendering with GLM matrices. The scene is
built around these spaces:

- Object space: coordinates stored in each mesh or model.
- World space: object coordinates after the object's model matrix is applied.
- View space: world coordinates as seen from the camera.
- Clip space: projected coordinates produced by the projection matrix.

The important matrix chain is:

```text
clipPosition = projection * view * model * objectPosition
```

The code keeps this chain explicit so it is easy to see where each transform is
introduced:

- `scene::Character`, `scene::FloatingCubes`, and `scene::VrmCharacter` build
  per-object model transforms.
- `scene::OrbitCamera` builds the view matrix from the camera orbit angles and
  radius.
- `app::App` owns the render loop and passes the view/projection matrices into
  scene objects.
- `assets/shaders/color.vert` applies model, view, projection, and optional
  skinning transforms.

The axis helper uses the common OpenGL convention in this sample: red is X,
green is Y, and blue is Z. Y is the vertical direction in the scene.

## Scene Objects

The scene is intentionally split into small classes:

- `scene::GridFloor`: checkerboard floor mesh.
- `scene::AxisGizmo`: RGB axis lines.
- `scene::FloatingCubes`: randomly colored cubes with smooth wandering motion.
- `scene::LightMarker`: visible sphere-like marker for the moving light.
- `scene::VrmCharacter`: VRM model wrapper used as the keyboard-controlled
  character.
- `scene::OrbitCamera`: mouse-controlled camera.
- `scene::CharacterController`: keyboard movement and rotation.

Drawable scene objects implement `scene::IDrawable`. Objects that can contribute
to the projected floor shadow implement `scene::IShadowCaster`. This mirrors the
old sample's class-based style while keeping the OpenGL-specific rendering code
in smaller render classes.

## Rendering Order

`app::App` coordinates the render order. The sample uses a straightforward
forward-rendered pipeline:

1. Clear the framebuffer and update the camera.
2. Draw the floor.
3. Draw planar shadows on the floor.
4. Draw the character, cubes, axes, and light marker.
5. Draw the character outline.
6. Draw the optional Dear ImGui debug UI.

The floor shadow is intentionally simple. It projects object geometry onto the
Y=0 floor plane from the current light position, clips the result to the floor
area, and draws it as translucent dark geometry.

## Shaders

Shader sources live under `assets/shaders`, but they are not read from disk at
runtime. CMake runs `cmake/GenerateShaderSources.cmake` and generates a C++
header under the build directory. This keeps shader text editable while avoiding
runtime asset lookup for the shader files.

`render::ShaderProgram` owns shader compilation, program linking, and uniform
lookup helpers. `render::ColoredMesh` and `render::ModelMesh` use that shader
program to draw simple geometry and loaded model primitives.

## Model Loading

`asset::GltfModelLoader` reads glTF/GLB/VRM data through cgltf and converts it
into `asset::ModelData`. The current loader supports the features needed by the
sample character:

- indexed triangle primitives
- `POSITION`, `NORMAL`, `TEXCOORD_0`, and `COLOR_0`
- `JOINTS_0` and `WEIGHTS_0` for skinning
- base color factors and base color textures
- embedded and external PNG/JPEG texture data through stb_image
- alpha modes and double-sided materials
- texture sampler state
- `KHR_texture_transform`
- scene nodes and parent/child hierarchy
- glTF skins and inverse bind matrices
- VRM 0.x humanoid bone lookup for selected bones

`asset::Model` owns the OpenGL-side representation. It uploads mesh data,
textures, and model primitives, then evaluates node transforms before drawing.

## Skinning

Skinning is the path that allows the character to move away from the default
T-pose. Each skinned vertex stores up to four joint indices and four weights:

- `JOINTS_0`: which bones affect the vertex.
- `WEIGHTS_0`: how much each bone affects the vertex.

For each frame, `asset::Model` evaluates the node hierarchy, builds joint
matrices, and sends them to the vertex shader. The key relationship is:

```text
jointMatrix = inverseMeshTransform * jointWorldTransform * inverseBindMatrix
```

The vertex shader blends up to four joint matrices per vertex using the vertex
weights. A vertex on an upper arm, for example, can move when the upper-arm bone
rotates because the blended skinning matrix changes its final position.

The implementation currently uses a fixed uniform array for joint matrices. That
is simple and readable for this sample. Larger production renderers often move
bone matrices to uniform buffers, shader storage buffers, or textures.

## Pose Animation

`asset::ModelPoseAnimator` applies a small procedural pose animation to the
loaded VRM model. It is not a full animation system. Instead, it demonstrates the
basic idea of editing node local transforms before the model evaluates world
transforms and skinning matrices.

The animator currently handles:

- upper-arm pose based on VRM humanoid bone names
- simple arm spreading animation
- optional head yaw
- optional tail swing based on tail node names

The Dear ImGui debug panel exposes the animation settings so the effect of each
parameter can be inspected while the scene is running.

## Lighting, Shadows, and Outline

Lighting is intentionally simple. A moving light position is used by the shader
and visualized by `scene::LightMarker`. This keeps the relationship between
light position, shaded geometry, and projected shadow visible.

The outline effect is also simple: the model is drawn again with vertices
expanded along their normals and a solid outline color. This is not a complete
toon renderer, but it gives the VRM character a clearer silhouette and keeps the
technique easy to inspect.

## Where to Read First

A good reading order is:

1. `src/main.cpp`
2. `include/matrixalchemy/app/App.hpp`
3. `src/app/App.cpp`
4. `include/matrixalchemy/scene/IDrawable.hpp`
5. `include/matrixalchemy/scene/IShadowCaster.hpp`
6. `src/scene/GridFloor.cpp`
7. `src/scene/FloatingCubes.cpp`
8. `src/scene/VrmCharacter.cpp`
9. `src/asset/GltfModelLoader.cpp`
10. `src/asset/Model.cpp`
11. `src/asset/ModelPoseAnimator.cpp`
12. `assets/shaders/color.vert`
13. `assets/shaders/color.frag`

This order starts with the application loop, then moves through simple scene
objects, model loading, skinning, pose animation, and finally shader behavior.
