#pragma once

#include <memory>
#include "Scene/BaseScene.h"
#include "../../GameObject/Player/Player.h"

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
  // std::unique_ptr<Player> player_;
};
