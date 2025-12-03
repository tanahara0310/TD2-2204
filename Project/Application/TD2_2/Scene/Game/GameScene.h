#pragma once

#include <vector>
#include <list>
#include <memory>
#include "Scene/BaseScene.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Boss/Boss.h"
#include "../../GameObject/Background/Background.h"
#include "../../GameObject/Frame/Frame.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/Bullet/Bullet.h"
#include "../../Collider/CollisionManager.h"
#include "../../Collider/CollisionConfig.h"
#include "../../Camera/CameraController.h"
#include "../../AI/BehaviorTree/BehaviorTree.h"
#include "../../Effect/Lightning/LightningEffectManager.h"

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

   /// @brief 弾を生成
   /// @param position 生成位置
   /// @param direction 進行方向
   /// @param speed 速度（デフォルト: 30.0f）
   /// @return 生成された弾のポインタ
   Bullet* CreateBullet(const Vector3& position, const Vector3& direction, float speed = 30.0f);

private:
   Player* player_;
   Boss* boss_;
   Background* background_;
   std::list<Bullet*> bullets_;
   std::list<Frame*> frames_;

   std::unique_ptr<CollisionManager> collisionManager_;
   std::unique_ptr<CollisionConfig> collisionConfig_;

   std::unique_ptr<BehaviorTree> bossBehaviorTree_;

   std::unique_ptr<CameraController> cameraController_;
   
   // 雷エフェクトマネージャー
   std::unique_ptr<LightningEffectManager> lightningEffectManager_;
   int playerDamageEffectId_ = -1;

private:
   void RegisterAllColliders();

   void CheckCollisions();

   std::unique_ptr<BehaviorTree> CreateBossBehaviorTree();

   void InitializeFrames();
};
