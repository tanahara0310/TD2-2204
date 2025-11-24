#pragma once

#include <vector>
#include <list>
#include <memory>
#include "Scene/BaseScene.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Boss/Boss.h"
#include "../../GameObject/GameObject.h"
#include "../../Collider/CollisionManager.h"
#include "../../Collider/CollisionConfig.h"

class EngineSystem;
class CameraManager;
struct DirectionalLightData;

/// @brief ゲームシーンクラス
class GameScene : public BaseScene {
public:
   /// @brief 初期化
   void Initialize(EngineSystem* engine) override;

   /// @brief 更新
   void Update() override;

   /// @brief 描画処理
   void Draw() override;

   /// @brief 解放
   void Finalize() override;

private:
   Player* player_;
   Boss* boss_;

   std::unique_ptr<CollisionManager> collisionManager_;
   std::unique_ptr<CollisionConfig> collisionConfig_;

private:
   void RegisterAllColliders();

   void CheckCollisions();
};
