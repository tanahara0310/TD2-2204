# オブジェクト描画フロー

## 概要
エンジンでは `RenderManager` を中心とした統一描画システムを採用しており、3Dモデル、スプライト、スキニングモデルなどを一括で効率的に描画します。

---

## 描画パスの種類

### RenderPassType 列挙型
```cpp
enum class RenderPassType {
    Invalid = -1,        // 無効
    Model = 0,           // 通常モデル
    SkinnedModel,        // スキニングモデル
    SkyBox,              // SkyBox
    Sprite,              // スプライト
};
```

---

## 描画可能オブジェクトの要件

### IDrawable インターフェース
全ての描画可能オブジェクトは `IDrawable` を継承する必要があります。

```cpp
class IDrawable {
public:
    virtual RenderPassType GetRenderPassType() const = 0;  // 描画パスタイプ
    virtual const char* GetObjectName() const = 0;         // オブジェクト名
    virtual bool DrawImGui() = 0;                          // デバッグUI
    virtual bool Is2D() const = 0;                         // 2D/3D判定
    virtual bool IsActive() const = 0;                     // アクティブ状態
};
```

---

## オブジェクトクラスの実装

### 3Dオブジェクト（Object3d）

#### 必須メンバ
- `std::unique_ptr<Model> model_` - モデルインスタンス
- `WorldTransform transform_` - ワールドトランスフォーム
- `TextureManager::LoadedTexture texture_` - テクスチャハンドル（基底クラスに含まれる）

#### 必須実装メソッド
```cpp
class MyObject : public Object3d {
public:
    void Initialize(EngineSystem* engine) override;
    void Update() override;
    void Draw(ICamera* camera) override;
    RenderPassType GetRenderPassType() const override;
};
```

#### 初期化の流れ
```cpp
void MyObject::Initialize(EngineSystem* engine) {
    // 1. ModelManagerからモデルを作成
    auto modelManager = engine->GetComponent<ModelManager>();
    model_ = modelManager->CreateStaticModel("path/to/model.obj");
    
    // 2. Transformの初期化（GPU用リソース作成）
    auto dxCommon = engine->GetComponent<DirectXCommon>();
    transform_.Initialize(dxCommon->GetDevice());
    
    // 3. 初期パラメータ設定
    transform_.translate = { 0.0f, 0.0f, 0.0f };
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    
    // 4. テクスチャを初期化時に読み込む（重要）
    texture_ = TextureManager::GetInstance().Load("texture.png");
}
```

#### 更新処理
```cpp
void MyObject::Update() {
    // トランスフォームをGPUに転送
    transform_.TransferMatrix();
}
```

#### 描画処理
```cpp
void MyObject::Draw(ICamera* camera) {
    if (!model_ || !camera) return;
    
    // モデル描画（初期化時に読み込んだテクスチャを使用）
    model_->Draw(transform_, camera, texture_.gpuHandle);
}
```

---

### スキニングモデル（SkeletonModelObject）

#### 追加メンバ
```cpp
private:
    EngineSystem* engine_ = nullptr;
    bool drawSkeleton_ = true;
    float jointRadius_ = 0.05f;
    TextureManager::LoadedTexture uvCheckerTexture_;  // テクスチャハンドル
```

#### 初期化の特殊性
```cpp
void SkeletonModelObject::Initialize(EngineSystem* engine) {
    engine_ = engine;
    auto modelManager = engine->GetComponent<ModelManager>();
    
    // 1. アニメーション事前読み込み
    AnimationLoadInfo animInfo;
    animInfo.directory = "path/to/model";
    animInfo.modelFilename = "model.gltf";
    animInfo.animationName = "animationName";
    animInfo.animationFilename = "model.gltf";
    modelManager->LoadAnimation(animInfo);
    
    // 2. スケルトンモデル作成
    model_ = modelManager->CreateSkeletonModel(
        "path/to/model.gltf",
        "animationName",
        true  // ループ再生
    );
    
    // 3. Transform初期化
    auto dxCommon = engine->GetComponent<DirectXCommon>();
    transform_.Initialize(dxCommon->GetDevice());
    
    // 4. テクスチャを初期化時に読み込む（重要）
    uvCheckerTexture_ = TextureManager::GetInstance().Load("texture.png");
}
```

#### アニメーション更新
```cpp
void SkeletonModelObject::Update() {
    // フレームレート取得
    auto frameRateController = engine_->GetComponent<FrameRateController>();
    float deltaTime = frameRateController->GetDeltaTime();
    
    // アニメーション更新（スケルトンも自動更新）
    if (model_->HasAnimationController()) {
        model_->UpdateAnimation(deltaTime);
    }
    
    // Transform更新
    transform_.TransferMatrix();
}
```

#### 描画処理
```cpp
void SkeletonModelObject::Draw(ICamera* camera) {
    if (!model_ || !camera) return;
    
    // 初期化時に読み込んだテクスチャを使用
    model_->Draw(transform_, camera, uvCheckerTexture_.gpuHandle);
}
```

---

### 2Dスプライト（SpriteObject）

#### 必須メンバ
```cpp
private:
    EngineSystem* engine_ = nullptr;
    std::unique_ptr<Sprite> sprite_;
    TextureManager::LoadedTexture textureHandle_;
```

#### 初期化
```cpp
void SpriteObject::Initialize(EngineSystem* engine, const std::string& texturePath) {
    engine_ = engine;
    
    // 1. Sprite初期化
    sprite_ = std::make_unique<Sprite>();
    auto* renderManager = engine->GetComponent<RenderManager>();
    auto* spriteRenderer = dynamic_cast<SpriteRenderer*>(
        renderManager->GetRenderer(RenderPassType::Sprite)
    );
    sprite_->Initialize(spriteRenderer, texturePath);
    
    // 2. テクスチャを初期化時に読み込む（重要）
    textureHandle_ = TextureManager::GetInstance().Load(texturePath);
    
    // 3. トランスフォーム初期化
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
}
```

#### 更新・描画
```cpp
void SpriteObject::Update() {
    // Spriteのトランスフォーム更新
    sprite_->SetPosition(transform_.translate);
    sprite_->SetScale(transform_.scale);
    sprite_->SetRotate(transform_.rotate);
}

void SpriteObject::Draw() {
    if (!sprite_) return;
    
    // 初期化時に読み込んだテクスチャを使用
    sprite_->Draw(textureHandle_.gpuHandle);
}
```

---

## シーンでの描画フロー

### TestScene::Initialize() - オブジェクト生成
```cpp
void TestScene::Initialize(EngineSystem* engine) {
    engine_ = engine;
    
    // 1. カメラマネージャーの初期化
    auto dxCommon = engine->GetComponent<DirectXCommon>();
    
    auto debugCamera = std::make_unique<DebugCamera>();
    debugCamera->Initialize(engine, dxCommon->GetDevice());
    cameraManager_->RegisterCamera("Debug", std::move(debugCamera));
    cameraManager_->SetActiveCamera("Debug");
    
    // 2. 3Dオブジェクトの生成と初期化
    auto skeletonModel = std::make_unique<SkeletonModelObject>();
    skeletonModel->Initialize(engine);
    skeletonModel->SetActive(true);
    gameObjects_.push_back(std::move(skeletonModel));
    
    // 3. スプライトオブジェクトの生成と初期化
    auto sprite = std::make_unique<SpriteObject>();
    sprite->Initialize(engine, "Resources/uvChecker.png");
    sprite->GetSprite()->SetPosition({ 100.0f, 100.0f, 0.0f });
    spriteObjects_.push_back(std::move(sprite));
}
```

### TestScene::Update() - オブジェクト更新
```cpp
void TestScene::Update() {
    // 1. カメラマネージャーの更新
    cameraManager_->Update();
    
    // 2. 3Dゲームオブジェクトの更新
    for (auto& obj : gameObjects_) {
        if (obj && obj->IsActive()) {
            obj->Update();
        }
    }
    
    // 3. ライトマネージャーの更新
    auto lightManager = engine_->GetComponent<LightManager>();
    if (lightManager) {
        lightManager->UpdateAll();
    }
}
```

### TestScene::Draw() - 統一描画システム
```cpp
void TestScene::Draw() {
    // 1. 必須コンポーネント取得
    auto renderManager = engine_->GetComponent<RenderManager>();
    auto dxCommon = engine_->GetComponent<DirectXCommon>();
    ICamera* activeCamera = cameraManager_->GetActiveCamera();
    
    if (!renderManager || !dxCommon || !activeCamera) {
        return;
    }
    
    ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();
    
    // 2. フレーム開始時の設定
    renderManager->SetCamera(activeCamera);
    renderManager->SetCommandList(cmdList);
    
    // 3. 全オブジェクトを描画キューに追加
    for (const auto& obj : gameObjects_) {
        if (obj && obj->IsActive()) {
            renderManager->AddDrawable(obj.get());
        }
    }
    
    for (const auto& spriteObj : spriteObjects_) {
        if (spriteObj && spriteObj->IsActive()) {
            renderManager->AddDrawable(spriteObj.get());
        }
    }
    
    // 4. 一括描画（自動的にパスごとにソート・描画）
    renderManager->DrawAll();
    
    // 5. フレーム終了時にキューをクリア
    renderManager->ClearQueue();
}
```

---

## RenderManager の内部動作

### 描画キューのソート
```cpp
void RenderManager::SortDrawQueue() {
    // 描画パスタイプでソート（パイプライン切り替え最小化）
    std::sort(drawQueue_.begin(), drawQueue_.end(),
        [](const DrawCommand& a, const DrawCommand& b) {
            return static_cast<int>(a.passType) < static_cast<int>(b.passType);
        });
}
```

### 描画実行
```cpp
void RenderManager::DrawAll() {
    if (drawQueue_.empty() || !cmdList_) return;
    
    SortDrawQueue();
    
    RenderPassType currentPass = RenderPassType::Invalid;
    
    for (const auto& cmd : drawQueue_) {
        // パス切り替え時の処理
        if (cmd.passType != currentPass) {
            // 前のパス終了
            if (currentPass != RenderPassType::Invalid) {
                renderers_[currentPass]->EndPass();
            }
            
            // 新しいパス開始
            currentPass = cmd.passType;
            renderers_[currentPass]->BeginPass(cmdList_, BlendMode::kBlendModeNone);
        }
        
        // オブジェクト描画
        if (cmd.object->Is2D()) {
            static_cast<Object2d*>(cmd.object)->Draw();
        } else {
            static_cast<Object3d*>(cmd.object)->Draw(camera_);
        }
    }
    
    // 最後のパス終了
    if (currentPass != RenderPassType::Invalid) {
        renderers_[currentPass]->EndPass();
    }
}
```

---

## テクスチャの正しい扱い方

### ? 間違った実装（Draw内でロード）
```cpp
void MyObject::Draw(ICamera* camera) {
    // 毎フレームロードするのは無駄！
    auto texture = TextureManager::GetInstance().Load("texture.png");
    model_->Draw(transform_, camera, texture.gpuHandle);
}
```

### ? 正しい実装（初期化時にロード）
```cpp
class MyObject : public Object3d {
private:
    TextureManager::LoadedTexture texture_;  // メンバ変数に保存
};

void MyObject::Initialize(EngineSystem* engine) {
    // ...モデル作成など...
    
    // 初期化時に1回だけロード
    texture_ = TextureManager::GetInstance().Load("texture.png");
}

void MyObject::Draw(ICamera* camera) {
    // 保存済みのハンドルを使用
    model_->Draw(transform_, camera, texture_.gpuHandle);
}
```

---

## まとめ

### 3Dモデルを描画するために必要なもの
1. `Object3d` を継承したクラス
2. `std::unique_ptr<Model> model_` メンバ
3. `WorldTransform transform_` メンバ
4. `TextureManager::LoadedTexture texture_` メンバ（基底クラスにある）
5. `Initialize()` で以下を実行：
   - モデル作成
   - Transform初期化
   - **テクスチャを読み込んでメンバ変数に保存**
6. `Update()` で `transform_.TransferMatrix()` 呼び出し
7. `Draw()` で保存済みテクスチャハンドルを使用して `model_->Draw()` 呼び出し
8. `GetRenderPassType()` で適切なパスタイプを返す

### スキニングモデルを描画するために必要なもの
1. 3Dモデルの要件に加えて：
2. `EngineSystem* engine_` メンバ（FrameRateController取得用）
3. `TextureManager::LoadedTexture uvCheckerTexture_` などの専用テクスチャメンバ
4. `Initialize()` で以下を実行：
   - アニメーション事前読み込み
   - スケルトンモデル作成
   - **テクスチャを読み込んでメンバ変数に保存**
5. `Update()` で以下を実行：
   - `model_->UpdateAnimation(deltaTime)` でアニメーション更新
   - `transform_.TransferMatrix()` でトランスフォーム更新
6. `Draw()` で保存済みテクスチャハンドルを使用

### スプライトを描画するために必要なもの
1. `Object2d` を継承したクラス
2. `std::unique_ptr<Sprite> sprite_` メンバ
3. `TextureManager::LoadedTexture textureHandle_` メンバ
4. `Initialize()` で以下を実行：
   - Sprite初期化
   - **テクスチャを読み込んでメンバ変数に保存**
5. `Update()` でSpriteのトランスフォーム更新
6. `Draw()` で保存済みテクスチャハンドルを使用して `sprite_->Draw()` 呼び出し
7. `GetRenderPassType()` で `RenderPassType::Sprite` を返す

### シーンでの描画手順
1. `Initialize()`: オブジェクト生成と初期化
2. `Update()`: オブジェクト更新
3. `Draw()`:
   - `RenderManager::SetCamera()` でカメラ設定
   - `RenderManager::SetCommandList()` でコマンドリスト設定
   - `RenderManager::AddDrawable()` で全オブジェクトをキューに追加
   - `RenderManager::DrawAll()` で一括描画
   - `RenderManager::ClearQueue()` でキューをクリア

### パフォーマンス最適化のポイント
- **テクスチャは初期化時に1回だけロード**し、メンバ変数に保存
- Draw()メソッド内で毎フレームロードしない
- RenderManagerが自動的に描画パスごとにソートして効率的に描画
