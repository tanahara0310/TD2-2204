#include "ResultScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Light/LightData.h"
#include "MathCore.h"

void ResultScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

   // リザルト画像の生成
   //resultSprite_ = std::make_unique<Sprite>();
   //resultSprite_->Initialize(engine_->GetComponent<SpriteRenderer>(), "Resources/Textures/white.png");
   //resultTexture_ = TextureManager::GetInstance().Load("Resources/Textures/Result/result.png");
}

void ResultScene::Update() {
   BaseScene::Update();
}

void ResultScene::Draw() {
   BaseScene::Draw();

   // リザルト画像の描画
   //resultSprite_->Draw(resultTexture_);
}

void ResultScene::Finalize() {
}
