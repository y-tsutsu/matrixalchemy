# Graphics Notes 日本語版

このドキュメントは、Matrix Alchemyの現在のレンダリングサンプルを読むためのガイドです。
実装の中で確認できる3Dグラフィックスの考え方と、その考え方が現れているソースファイルを整理しています。

## ソース構成

プロジェクトは役割ごとに分かれています。

- `app`: アプリケーションの寿命、入力コールバック、更新ループ、描画順序。
- `scene`: 床、カメラ、キューブ、ライトマーカー、キャラクターなどのシーン内オブジェクト。
- `render`: メッシュ、テクスチャ、シェーダープログラム、影行列ヘルパーなどのOpenGLラッパー。
- `asset`: glTF/VRM読み込み、モデルデータ変換、スキニングデータ、ポーズアニメーション。
- `ui`: Dear ImGuiのデバッグ操作。

中心になる流れは次の通りです。

```text
main.cpp
  -> app::App::run()
  -> app::App::update()
  -> app::App::render()
  -> scene objects
  -> render wrappers and shaders
```

## 座標系と行列

Matrix Alchemyは、GLMの行列を使ったOpenGLスタイルの3Dレンダリングを行います。
シーンは次の空間を基準に組み立てられています。

- オブジェクト空間: 各メッシュやモデルに保存されている座標。
- ワールド空間: オブジェクトのモデル行列を適用した後の座標。
- ビュー空間: カメラから見たワールド座標。
- クリップ空間: 射影行列によって作られる投影後の座標。

重要な行列の連鎖は次の形です。

```text
clipPosition = projection * view * model * objectPosition
```

コードではこの連鎖を明示的に保っているので、それぞれの変換がどこで入るかを追いやすくしています。

- `scene::Character`、`scene::FloatingCubes`、`scene::VrmCharacter`は、各オブジェクトのモデル変換を作ります。
- `scene::OrbitCamera`は、カメラの軌道角度と半径からビュー行列を作ります。
- `app::App`はレンダーループを持ち、ビュー行列と射影行列をシーンオブジェクトへ渡します。
- `assets/shaders/color.vert`は、モデル、ビュー、射影、必要ならスキニング変換を適用します。

軸表示は、このサンプルで使っている一般的なOpenGL寄りの約束に従っています。
赤がX、緑がY、青がZです。シーンではYが上下方向です。

読み始めるのに向いているコードは次のあたりです。

- `scene::OrbitCamera::viewMatrix()` in `src/scene/OrbitCamera.cpp`
- `scene::FloatingCubes::modelMatrix()` in `src/scene/FloatingCubes.cpp`
- `scene::VrmCharacter::draw()` in `src/scene/VrmCharacter.cpp`
- `app::App::render()` in `src/app/App.cpp`
- `main()` in `src/main.cpp`

## シーンオブジェクト

シーンは意図的に小さなクラスへ分割しています。

- `scene::GridFloor`: チェッカーボード状の床メッシュ。
- `scene::AxisGizmo`: RGBの軸線。
- `scene::ArcaneRing`: キャラクターの足元で動く加算合成のラインリング。
- `scene::FloatingCubes`: ランダム色で滑らかに漂うキューブ群。
- `scene::LightMarker`: 動くライト位置を示す球状のマーカー。
- `scene::VrmCharacter`: キーボード操作キャラクターとして使うVRMモデルラッパー。
- `scene::OrbitCamera`: マウス操作のカメラ。
- `scene::CharacterController`: キーボード移動と回転。

描画できるシーンオブジェクトは`scene::IDrawable`を実装します。
床に投影する影を描けるオブジェクトは`scene::IShadowCaster`を実装します。
これは昔のサンプルのクラスベースな雰囲気を保ちつつ、OpenGL固有の描画コードを小さなrenderクラスへ分けるための構成です。

2つのシーンインターフェースは意図的に小さくしています。

- `IDrawable::draw()`は、現在有効なシェーダーで自分自身を描画できることを表します。
- `IShadowCaster::drawShadow()`は、影の投影行列を通して自分自身を描画できることを表します。

これにより、フル機能のシーングラフやエンティティシステムを導入せず、元の学習用コードに近い読みやすさを保っています。

## 描画順序

`app::App`が描画順序を管理します。サンプルは素直なフォワードレンダリングの流れです。

1. フレームバッファをクリアし、カメラを更新する。
2. 床を描きながら、床領域をステンシルバッファへ書き込む。
3. ステンシルバッファ上で床になっている領域にだけ、平面投影の影を描く。
4. 軸、ライトマーカー、浮遊キューブ、キャラクターを描く。
5. キャラクターの描画パス内でアウトラインを描く。
6. 任意でDear ImGuiのデバッグUIを描く。

床の影は意図的にシンプルです。現在のライト位置からY=0の床平面へオブジェクト形状を投影し、床領域にクリップして、半透明の暗い形状として重ねています。

`app::App::render()`では、次のレンダーステート変更が読みどころです。

- `glStencilFunc`、`glStencilMask`、`glStencilOp`で影を床上に制限します。
- `glEnable(GL_BLEND)`と`glBlendFunc`で投影影を半透明にします。
- `glDepthMask(GL_FALSE)`で影パスが深度を書かないようにします。
- `uUseColorOverride`で、モデルのマテリアルとは独立した影色を強制します。

## シェーダー

シェーダーソースは`assets/shaders`以下にありますが、実行時にはファイルから読み込みません。
CMakeが`cmake/GenerateShaderSources.cmake`を実行し、ビルドディレクトリ内にC++ヘッダーを生成します。
これにより、シェーダー本文は編集しやすいファイルとして管理しつつ、実行時のシェーダーファイル探索を不要にしています。

`render::ShaderProgram`は、シェーダーのコンパイル、プログラムリンク、uniform設定ヘルパーを持ちます。
`render::ColoredMesh`と`render::ModelMesh`は、そのシェーダープログラムを使って単純なジオメトリや読み込んだモデルプリミティブを描画します。

頂点シェーダーは次の属性を使います。

- location 0: `aPosition`
- location 1: `aColor`
- location 2: `aTexCoord`
- location 3: `aNormal`
- location 4: `aJoints`
- location 5: `aWeights`

`render::ColoredMesh`は、床、軸、フォールバックキャラクター、ライトマーカー、キューブで使うシンプルな位置/色/法線データをアップロードします。
`render::ModelMesh`は、テクスチャ座標やスキニング属性も含むモデル用の頂点形式をアップロードします。

## モデル読み込み

`asset::GltfModelLoader`はcgltfを使ってglTF/GLB/VRMデータを読み、`asset::ModelData`へ変換します。
現在のローダーは、サンプルキャラクターに必要な次の機能をサポートしています。

- インデックス付き三角形プリミティブ
- `POSITION`、`NORMAL`、`TEXCOORD_0`、`COLOR_0`
- スキニング用の`JOINTS_0`と`WEIGHTS_0`
- ベースカラー係数とベースカラーテクスチャ
- stb_imageによる埋め込み/外部PNG/JPEGテクスチャデータ
- アルファモードと両面マテリアル
- テクスチャサンプラー状態
- `KHR_texture_transform`
- シーンノードと親子階層
- glTFのスキンと逆バインド行列
- 選択したボーンに対するVRM 0.x humanoidボーン検索
- VRM 0.x MToonの`_ShadeColor`、`_ShadeShift`、`_ShadeToony`、`_RimColor`、
  `_RimFresnelPower`、`_EmissionColor`、`_OutlineColor`、`_OutlineWidth`の読み取り

`asset::Model`はOpenGL側の表現を持ちます。メッシュデータ、テクスチャ、モデルプリミティブをアップロードし、描画前にノード変換を評価します。

モデル読み込みのデータフローは次の通りです。

```text
cgltf data
  -> asset::GltfModelLoader
  -> asset::ModelData
  -> asset::Model
  -> render::ModelMesh / render::Texture2D
```

`asset::ModelData`は、読み込みと描画の間でデータを受け渡す構造体です。

- `ModelPrimitive`: 1つのプリミティブに対するCPU側の頂点とマテリアルフラグ。
- `ToonMaterial`: サンプルのtoonシェーダーとアウトラインパスで使う、VRM 0.x MToon値の小さなサブセット。
- `ModelInstance`: シーンノードに接続されたプリミティブ。必要ならスキンも持ちます。
- `ModelNode`: ローカル/ワールド変換と親子関係。
- `ModelSkin`: jointノードインデックスと逆バインド行列。
- `textures`: デコードされ、アップロードされたテクスチャオブジェクト。

## スキニング

スキニングは、キャラクターをデフォルトのTポーズから動かすための経路です。
スキニングされた各頂点は、最大4つのjointインデックスと4つの重みを持ちます。

- `JOINTS_0`: その頂点に影響するボーン。
- `WEIGHTS_0`: 各ボーンがどれくらい影響するか。

各フレームで`asset::Model`はノード階層を評価し、joint行列を作って頂点シェーダーへ送ります。
重要な関係は次の通りです。

```text
jointMatrix = inverseMeshTransform * jointWorldTransform * inverseBindMatrix
```

頂点シェーダーは、頂点の重みに従って最大4つのjoint行列をブレンドします。
たとえば上腕上の頂点は、上腕ボーンが回転すると、ブレンドされたスキニング行列が変わるため最終位置も動きます。

現在の実装では、joint行列に固定長のuniform配列を使っています。
このサンプルでは十分シンプルで読みやすい方法です。
より大きなプロダクション向けレンダラーでは、uniform buffer、shader storage buffer、テクスチャなどへ移すことがあります。

重要な実装箇所は次の通りです。

- `GltfModelLoader.cpp`が`JOINTS_0`、`WEIGHTS_0`、スキン、逆バインド行列を読みます。
- `ModelMesh.cpp`はjointインデックスを`glVertexAttribIPointer`でバインドします。jointインデックスは整数の頂点属性だからです。
- `Model::jointMatrices()`がシェーダーへ送る行列配列を作ります。
- `assets/shaders/color.vert`がjoint行列をブレンドし、通常のアウトライン、モデル、ビュー、射影変換の前に適用します。

現在のサンプルでは、シェーダー内のjoint行列を128個に制限しています。
サンプルキャラクターには十分で、uniformを使った経路を読みやすく保てます。

## ポーズアニメーション

`asset::ModelPoseAnimator`は、読み込んだVRMモデルへ小さな手続き的ポーズアニメーションを適用します。
これはフル機能のアニメーションシステムではありません。
モデルがワールド変換とスキニング行列を評価する前に、ノードのローカル変換を編集するという基本的な考え方を見せるためのものです。

現在のアニメーターは次を扱います。

- VRM humanoidボーン名に基づく上腕ポーズ
- 両腕を広げる簡単なアニメーション
- 任意の頭のヨー回転
- tailノード名に基づく任意の尻尾揺れ

Dear ImGuiのデバッグパネルでは、各パラメータの効果を実行中に確認できるように、アニメーション設定を公開しています。

ポーズデータの流れは次の通りです。

```text
DebugUi sliders
  -> app::App::poseAnimationSettings()
  -> scene::VrmCharacter::update()
  -> asset::Model::applyDemoPose()
  -> asset::ModelPoseAnimator::apply()
  -> node local transforms
  -> skinning matrices
```

この仕組みは、アニメーションをクリッププレイヤーの内側に隠さないので学習しやすいです。
コードが直接いくつかのローカルノード変換を変更し、その後は通常のノード階層とスキニング処理に任せています。

## ライティング、影、アウトライン

ライティングは意図的にシンプルです。
動くライト位置をシェーダーで使い、`scene::LightMarker`で可視化します。
これにより、ライト位置、陰影、投影影の関係を見やすくしています。

VRMキャラクターは、フラグメントシェーダー内で小さなtoonライティングパスを使います。
これは完全なMToon実装ではありませんが、ワールド法線、ライト位置、half-Lambertライティング、shade colorのブレンドを使って、モデルがtoonキャラクターらしく見えるようにしています。
VRM 0.xマテリアルがMToonの`_ShadeColor`を持っている場合、その色をマテリアルのshade colorとして使えます。
デバッグUIではマテリアルのshade使用を無効にし、1つのグローバルなshade colorを使うこともできます。
`_ShadeShift`は明暗境界を動かし、`_ShadeToony`は境界をよりくっきりさせます。
`_RimColor`と`_RimFresnelPower`は視線方向に依存した小さなリムライトを加え、`_EmissionColor`はマテリアルへ非ライティングの色成分を加えます。

これらの値は`asset::ToonMaterial`に保存されます。
レンダラーはそれらを任意のマテリアルヒントとして扱います。
値がない場合や対応するデバッグトグルがオフの場合、シェーダーはサンプル全体のグローバルなtoon lighting値へフォールバックします。

アウトラインもシンプルです。モデルをもう一度描き、頂点を法線方向へ広げ、単色のアウトライン色で描きます。
利用できる場合は、VRM 0.x MToonの`_OutlineColor`と`_OutlineWidth`をマテリアルごとに使います。
それらがないマテリアルは、サンプルのデフォルトアウトライン色と幅にフォールバックします。
これは完全なtoonレンダラーではありませんが、VRMキャラクターのシルエットを分かりやすくし、技法を追いやすくしています。

重要なファイルは次の通りです。

- `render::planarShadowMatrix()` in `src/render/Shadow.cpp`
- `scene::ArcaneRing::draw()` in `src/scene/ArcaneRing.cpp`
- `scene::LightMarker::update()` in `src/scene/LightMarker.cpp`
- `scene::VrmCharacter::drawShadow()` in `src/scene/VrmCharacter.cpp`
- `asset::Model::drawOutline()` in `src/asset/Model.cpp`
- `uOutlineWidth` handling in `assets/shaders/color.vert`

## Debug UI

Debug UIはconfigure時に任意で有効になります。
Dear ImGuiとGLFW/OpenGL3バックエンドが見つかると、CMakeが`MATRIXALCHEMY_HAS_IMGUI`を定義し、`src/ui/DebugUi.cpp`をコンパイルします。

デバッグパネルでは、学習中に便利な値を確認、調整できます。

- カメラの半径、theta、phi
- キャラクター位置、描画高さ、回転
- 浮遊キューブの回転
- 腕アニメーションの速度と角度
- 頭のヨー量
- 尻尾の揺れ量
- toon lightingのON/OFF、MToonマテリアルトグル、shade color、threshold、softness、lit strength

このパネルはシーン状態を直接所有せず、`app::App`のアクセサに接続されています。
これにより、Debug UIはシーンのもう1つの所有者ではなく、調査と操作のための層として保たれます。

## どこから読むとよいか

おすすめの読み順は次の通りです。

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

この順番なら、まずアプリケーションループから入り、シンプルなシーンオブジェクト、モデル読み込み、スキニング、ポーズアニメーション、最後にシェーダーの挙動へ進めます。

その後、さらに深く読むなら次の箇所が役立ちます。

- Texture loading: `src/render/Texture2D.cpp`
- Shader compilation: `src/render/ShaderProgram.cpp`
- VRM humanoid lookup: `src/asset/GltfModelLoader.cpp`
- Debug controls: `src/ui/DebugUi.cpp`
- Generated shader embedding: `cmake/GenerateShaderSources.cmake`
