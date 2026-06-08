# Graphics Notes

This document is a reading guide for Matrix Alchemy's current rendering sample.
It focuses on the graphics concepts that are visible in the implementation and
points to the source files where each idea appears.

## Source Map

The project is grouped by responsibility:

- `app`: application lifetime, input callbacks, update loop, and render order.
- `scene`: objects that exist in the scene, such as the floor, camera, cubes,
  light marker, and character.
- `render`: OpenGL wrappers for meshes, textures, shader programs, and shadow
  matrix helpers.
- `asset`: glTF/VRM loading, model data conversion, skinning data, and pose
  animation.
- `ui`: Dear ImGui debug controls.

The central flow is:

```text
main.cpp
  -> app::App::run()
  -> app::App::update()
  -> app::App::render()
  -> scene objects
  -> render wrappers and shaders
```

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

Good code entry points:

- `scene::OrbitCamera::viewMatrix()` in `src/scene/OrbitCamera.cpp`
- `scene::FloatingCubes::modelMatrix()` in `src/scene/FloatingCubes.cpp`
- `scene::VrmCharacter::draw()` in `src/scene/VrmCharacter.cpp`
- `app::App::render()` in `src/app/App.cpp`
- `main()` in `src/main.cpp`

## Scene Objects

The scene is intentionally split into small classes:

- `scene::GridFloor`: checkerboard floor mesh.
- `scene::AxisGizmo`: RGB axis lines.
- `scene::ArcaneRing`: animated additive line ring around the character.
- `scene::FloatingCubes`: randomly colored cubes with smooth wandering motion.
- `scene::LightMarker`: visible sphere-like marker for the moving light.
- `scene::VrmCharacter`: VRM model wrapper used as the keyboard-controlled
  character.
- `scene::OrbitCamera`: mouse-controlled camera.
- `scene::CharacterController`: keyboard movement and rotation.

Drawable scene objects derive from `scene::SceneObject`. It keeps the common
scene lifecycle in one small base class:

- `SceneObject::draw()` is pure virtual, so every visible scene object must
  implement its own drawing path.
- `SceneObject::release()` is pure virtual, so OpenGL resources stay explicitly
  releasable while the context is alive.
- `SceneObject::update()` has an empty default for static objects.
- `SceneObject::drawShadow()` has an empty default for objects that do not
  contribute to the projected floor shadow.

This keeps the sample close to the original class-based learning style without
introducing a full scene graph or entity system. `app::App` owns the concrete
scene instances and registers the regular per-frame objects in draw order.

## Rendering Order

`app::App` coordinates the render order. The sample uses a straightforward
forward-rendered pipeline:

1. Clear the framebuffer and update the camera.
2. Draw the floor while writing the floor area to the stencil buffer.
3. Draw planar shadows only where the stencil buffer says the floor exists.
4. Draw axes, the light marker, floating cubes, and the character.
5. Draw the character outline as part of the character draw path.
6. Draw the Dear ImGui debug UI.

The floor shadow is intentionally simple. It projects object geometry onto the
Y=0 floor plane from the current light position, clips the result to the floor
area, and draws it as translucent dark geometry.

The render-state changes are worth reading in `app::App::render()`:

- `glStencilFunc`, `glStencilMask`, and `glStencilOp` keep shadows on the floor.
- `glEnable(GL_BLEND)` and `glBlendFunc` make the projected shadow translucent.
- `glDepthMask(GL_FALSE)` prevents the shadow pass from writing depth.
- `uUseColorOverride` forces the shadow color independent of model materials.

## Shaders

Shader sources live under `assets/shaders`, but they are not read from disk at
runtime. CMake runs `cmake/GenerateShaderSources.cmake` and generates a C++
header under the build directory. This keeps shader text editable while avoiding
runtime asset lookup for the shader files.

`render::ShaderProgram` owns shader compilation, program linking, and uniform
lookup helpers. `render::ColoredMesh` and `render::ModelMesh` use that shader
program to draw simple geometry and loaded model primitives.

The vertex shader uses these attributes:

- location 0: `aPosition`
- location 1: `aColor`
- location 2: `aTexCoord`
- location 3: `aNormal`
- location 4: `aJoints`
- location 5: `aWeights`

`render::ColoredMesh` uploads the simpler position/color/normal data used by the
floor, axes, fallback character, light marker, and cubes. `render::ModelMesh`
uploads the full model vertex format, including texture coordinates and skinning
attributes.

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
- VRM 0.x MToon `_ShadeColor`, `_ShadeShift`, `_ShadeToony`, `_RimColor`,
  `_RimFresnelPower`, `_EmissionColor`, `_OutlineColor`, and `_OutlineWidth`
  lookup for toon lighting and outline rendering

`asset::Model` owns the OpenGL-side representation. It uploads mesh data,
textures, and model primitives, then evaluates node transforms before drawing.

The model loading data flow is:

```text
cgltf data
  -> asset::GltfModelLoader
  -> asset::ModelData
  -> asset::Model
  -> render::ModelMesh / render::Texture2D
```

`asset::ModelData` is the handoff structure between loading and rendering:

- `ModelPrimitive`: CPU-side vertices and material flags for one primitive.
- `ToonMaterial`: the small subset of VRM 0.x MToon values used by the sample
  toon shader and outline pass.
- `ModelInstance`: a primitive attached to a scene node, optionally with a skin.
- `ModelNode`: local/world transforms and parent relationship.
- `ModelSkin`: joint node indices and inverse bind matrices.
- `textures`: decoded and uploaded texture objects.

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

The important implementation points are:

- `GltfModelLoader.cpp` reads `JOINTS_0`, `WEIGHTS_0`, skins, and inverse bind
  matrices.
- `ModelMesh.cpp` binds joint indices with `glVertexAttribIPointer` because
  joint indices are integer vertex attributes.
- `Model::jointMatrices()` builds the matrix array sent to the shader.
- `assets/shaders/color.vert` blends the joint matrices and applies the result
  before the normal outline and model/view/projection transform.

The current sample limits the shader to 128 joint matrices. This is enough for
the sample character and keeps the shader uniform path easy to follow.

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

The pose data flow is:

```text
DebugUi sliders
  -> app::App::poseAnimationSettings()
  -> scene::VrmCharacter::update()
  -> asset::Model::applyDemoPose()
  -> asset::ModelPoseAnimator::apply()
  -> node local transforms
  -> skinning matrices
```

This is useful for learning because the animation does not hide behind a clip
player. The code directly changes a few local node transforms, then lets the
normal node hierarchy and skinning path handle the result.

## Lighting, Shadows, and Outline

Lighting is intentionally simple. A moving light position is used by the shader
and visualized by `scene::LightMarker`. This keeps the relationship between
light position, shaded geometry, and projected shadow visible.

The VRM character and floating cubes use a small toon-lighting pass in the
fragment shader. It is not a full MToon implementation, but it uses the world
normal, light position, half-Lambert lighting, and a shade-color blend to make
the model and cubes read with clearer lit and shaded faces. When a VRM 0.x
material provides MToon `_ShadeColor`, that color can be used as the material
shade color. The debug UI can also disable material shade usage and use one
global shade color instead. `_ShadeShift` moves the light/shade boundary,
`_ShadeToony` makes the boundary sharper, `_RimColor` and `_RimFresnelPower` add
a small view-dependent rim light, and `_EmissionColor` adds an unlit color
contribution to the material.

These values are stored in `asset::ToonMaterial`. The renderer treats them as
optional material hints: when a value is missing or the matching debug toggle is
off, the shader falls back to the sample's global toon-lighting values.

The outline effect is also simple: the model is drawn again with vertices
expanded along their normals and a solid outline color. When available, VRM 0.x
MToon `_OutlineColor` and `_OutlineWidth` are used per material. Materials
without those settings fall back to the sample's default outline color and width.
This is not a complete toon renderer, but it gives the VRM character a clearer
silhouette and keeps the technique easy to inspect.

Important files:

- `render::planarShadowMatrix()` in `src/render/Shadow.cpp`
- `scene::ArcaneRing::draw()` in `src/scene/ArcaneRing.cpp`
- `scene::LightMarker::update()` in `src/scene/LightMarker.cpp`
- `scene::VrmCharacter::drawShadow()` in `src/scene/VrmCharacter.cpp`
- `asset::Model::drawOutline()` in `src/asset/Model.cpp`
- `uOutlineWidth` handling in `assets/shaders/color.vert`

## Debug UI

The debug UI is built as a required part of the application. CMake requires Dear
ImGui and its GLFW/OpenGL3 backends, then compiles `src/ui/DebugUi.cpp`.

The debug panel exposes values that are useful while learning:

- camera radius, theta, and phi
- character position, render height, and rotation
- floating cube rotation
- arm animation speed and angles
- head yaw amount
- tail swing amount
- toon lighting on/off, MToon material toggles, shade color, threshold,
  softness, and lit strength

The panel is intentionally connected to `app::App` accessors instead of owning
scene state directly. That keeps the debug UI as an inspection/control layer
rather than another owner of the scene.

## Where to Read First

A good reading order is:

1. `src/main.cpp`
2. `include/matrixalchemy/app/App.hpp`
3. `src/app/App.cpp`
4. `include/matrixalchemy/scene/SceneObject.hpp`
5. `src/scene/ArcaneRing.cpp`
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

After that, these deeper paths are useful:

- Texture loading: `src/render/Texture2D.cpp`
- Shader compilation: `src/render/ShaderProgram.cpp`
- VRM humanoid lookup: `src/asset/GltfModelLoader.cpp`
- Debug controls: `src/ui/DebugUi.cpp`
- Generated shader embedding: `cmake/GenerateShaderSources.cmake`
