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
	  // 球体オブジェクトの生成と初期化
	  auto sphere = std::make_unique<Sphere>();
	  sphere->Initialize();
	  gameObjects_.push_back(std::move(sphere));

   }
}

void TitleScene::Update() {
   BaseScene::Update();
}

void TitleScene::Draw() {
   BaseScene::Draw();
}

void TitleScene::Finalize() {
}
