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

   std::vector<std::unique_ptr<IDrawable>> sprites;

   // リザルト画像の生成
   auto resultSprite = CreateResultSprite();

   resultSprite_ = resultSprite.get();
   sprites.push_back(std::move(resultSprite));

   for (auto& sprite : sprites) {
	   gameObjects_.push_back(std::move(sprite));
   }
}

void ResultScene::Update() {
   BaseScene::Update();

   // リザルト画像の更新
   if (resultSprite_)
	   resultSprite_->Update();
}

void ResultScene::Draw() {
   BaseScene::Draw();
}

void ResultScene::Finalize() {}

std::unique_ptr<SpriteObject> ResultScene::CreateResultSprite() { 
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/Textures/white.png");
	sprite->GetTransform().translate = {640.0f, 360.0f, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite; 
}
