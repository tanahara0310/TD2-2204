#pragma once

#include <vector>
#include <list>
#include <memory>
#include <array>
#include "Scene/BaseScene.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/Boss/Boss.h"
#include "../../GameObject/Background/Background.h"
#include "../../GameObject/Frame/Frame.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/Bullet/Bullet.h"
#include "../../GameObject/Cloud/Cloud.h"
#include "../../GameObject/SparkColliderObject/SparkColliderObject.h"
#include "../../Collider/CollisionManager.h"
#include "../../Collider/CollisionConfig.h"
#include "../../Camera/CameraController.h"
#include "../../Camera/CinematicSequence.h"
#include "../../AI/BehaviorTree/BehaviorTree.h"
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "../../UI/HitPoint/HitPoint.h"
#include "Engine/Particle/ParticleSystem.h"
#include "../../Utility/StateMachine.h"

class EngineSystem;
class CameraManager;
struct DirectionalLightData;

enum class BulletType {
   LightningBullet,
   ElasticSphere
};

/// @brief ボスAIパラメータ構造体（調整用）
struct BossAIParameters {
   // 距離評価
   float maxCenterDistance = 30.0f;      // 中央距離の最大評価範囲
   float closeToPlayerDistance = 15.0f;  // プレイヤーに近いと判定する距離
   float closeRangeDistance = 20.0f;     // 近距離攻撃の判定距離
   float mediumRangeMin = 15.0f;         // 中距離の最小値
   float mediumRangeMax = 25.0f;         // 中距離の最大値
   float dangerZoneDistance = 8.0f;      // 壁との危険距離

   // HP3フェーズのウェイト
   struct HP3Weights {
      float chargeOnly = 1.2f;           // 突進のみ（近距離バイアス倍率）
      float chargeAndFlee = 0.5f;        // 突進→逃げ
      float doubleCharge = 0.4f;         // 突進×2
      float shootOnly = 0.4f;            // ショット
      float shootAndCharge = 0.35f;      // ショット→突進
      float moveCenterAndCharge = 0.3f;  // 中央移動→突進
   } hp3;

   // HP2フェーズのウェイト
   struct HP2Weights {
      float sparkCombo = 0.5f;           // スパークコンボ
      float sparkComboClose = 1.0f;      // スパークコンボ近距離バイアス
      float stunPursuitBias = 1.0f;      // スタン追撃バイアス
      float chargeAndFlee = 0.25f;       // 突進→逃げ
      float energyChargeCombo = 1.5f;    // エネルギー高時の突進コンボ
      float fleeWhenLowEnergy = 1.2f;    // エネルギー低時の逃げ
   } hp2;

   // HP1フェーズのウェイト
   struct HP1Weights {
      float centerBias = 1.5f;           // 中央確保バイアス
      float sparkComboCloseBias = 1.0f;  // スパークコンボ近距離バイアス
      float sparkComboNonStunBias = 0.8f;// スパークコンボ非スタンバイアス
      float shootMediumBias = 0.7f;      // ショット中距離バイアス
      float shootCenterBias = 0.5f;      // ショット中央バイアス
      float tripleChargeCloseBias = 1.2f;// 3連突進近距離バイアス
      float tripleChargeCenterBias = 0.8f;// 3連突進中央バイアス
      float quadChargeStunBias = 2.0f;   // 4連突進スタンバイアス（スパーク前隙専用）
      float feintShoot = 0.3f;           // フェイント→ショット
      float sparkComboFarBias = 0.6f;    // スパークコンボ遠距離バイアス
      float shootCharge = 0.25f;         // ショット→突進
      float retreatDoubleShoot = 0.2f;   // 逃げ→ショット2連
      float energyReadyCharge = 1.8f;    // エネルギー準備完了時の突進
      float safetyCharge = 1.5f;         // ピンチ時の安全突進
   } hp1;

   // スタン判定
   float stunBiasWhenStunned = 1.0f;     // スタン時の評価値
   float stunBiasWhenNotStunned = 0.1f;  // 非スタン時の評価値
};

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
   Background* background_;
   std::list<Bullet*> bullets_;
   std::list<Frame*> frames_;
   std::array<Cloud*, 4> clouds_;

   // スパーク当たり判定用オブジェクト
   SparkColliderObject* sparkCollider_ = nullptr;

   std::unique_ptr<CollisionManager> collisionManager_;
   std::unique_ptr<CollisionConfig> collisionConfig_;

   std::unique_ptr<BehaviorTree> bossBehaviorTree_;

   std::unique_ptr<CameraController> cameraController_;

   // 雷エフェクトマネージャー
   std::unique_ptr<LightningEffectManager> lightningManager_;

   std::vector<std::unique_ptr<IDrawable>> newGameObjectsQueue_;

   ParticleSystem* playerCollisionParticle_;
   ParticleSystem* bossExplosionParticle_;
   ParticleSystem* playerExplosionParticle_;
   ParticleSystem* playerSmokeParticle_;
   ParticleSystem* bossSmokeParticle_;

   std::unique_ptr<HitPoint> playerHitPointUI_;
   std::unique_ptr<HitPoint> bossHitPointUI_;

   SpriteObject* ui_;
   SpriteObject* startUI_;

   GameTimer uiAnimationTimer_;

   Sound bgmSound_;
   Sound hitSound_;
   Sound damageSound_;
   Sound chargeSound_;
   Sound biribiriSound_;

   std::unique_ptr<GaugeUI> playerGauge_;
   std::unique_ptr<GaugeUI> bossGauge_;

   // スパークエフェクトID
   int sparkEffectId_ = -1;

   float time_ = 0.0f;

   std::unique_ptr<StateMachine> stateMachine_;

   // カメラシーケンスのカット追跡用
   int lastCutIndex_ = -1;

   // ボスAIパラメータ
   BossAIParameters aiParams_;

private:
   void RegisterAllColliders();

   void CheckCollisions();

   std::unique_ptr<BehaviorTree> CreateBossBehaviorTree();

   void InitializeFrames();

   /// @brief 弾を生成
   /// @param position 生成位置
   /// @param direction 進行方向
   /// @param type 弾のタイプ
   /// @param speed 速度（デフォルト: 30.0f）
   /// @return 生成された弾のポインタ
   Bullet* CreateBullet(const Vector3& position, const Vector3& direction, BulletType type, float speed = 30.0f);

   void StartUIAnimation();

   /// @brief パーティクルシステムを生成
   /// @param presetPath プリセットファイルのパス
   /// @return 生成されたパーティクルシステム
   std::unique_ptr<ParticleSystem> CreateParticleSystem(const std::string& presetPath);

   /// @brief パーティクルを発生させる
   /// @param particleSystem パーティクルシステム
   /// @param position 発生位置
   void EmitParticle(ParticleSystem* particleSystem, const Vector3& position);

   /// @brief パーティクルシステムの自動非アクティブ化をチェック
   /// @param particleSystem パーティクルシステム
   void CheckParticleAutoDeactivate(ParticleSystem* particleSystem);

   void InitializeOpening();

   void Opening();

   void InitializeMain();

   void Main();

   void InitializeGameOver();

   void GameOver();

   void InitializeGameClear();

   void GameClear();
};
