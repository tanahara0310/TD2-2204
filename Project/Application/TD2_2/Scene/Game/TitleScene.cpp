#include "TitleScene.h"
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

void TitleScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

   // ゲームオブジェクトの初期化
   {
      // UI初期化
      titleUI_ = std::make_unique<TitleUI>();
      auto sprites = titleUI_->Initialize(engine);
      
      // スプライトをgameObjects_に追加
      for (auto& sprite : sprites) {
         gameObjects_.push_back(std::move(sprite));
      }
   }
}

void TitleScene::Update() {
   BaseScene::Update();
   
   // UI更新
   if (titleUI_) {
      titleUI_->Update();
   }
}

void TitleScene::Draw() {
   BaseScene::Draw();
}

void TitleScene::Finalize() {
}
