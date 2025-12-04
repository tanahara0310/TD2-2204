#pragma once

#include "IScene.h"
#include "Engine/Graphics/Light/LightData.h"
#include "ObjectCommon/IDrawable.h"
#include <memory>
#include <vector>

class EngineSystem;
class CameraManager;
class DirectXCommon;
class Object3d;
class RenderManager;
class LineRenderer;  // 前方宣言
class Camera;        // 前方宣言

/// @brief シーンの基底クラス（共通処理を実装）
class BaseScene : public IScene {
public:

   virtual ~BaseScene() = default;

   /// @brief 初期化（共通処理 + 派生クラスの初期化）
   virtual void Initialize(EngineSystem* engine) override;

   /// @brief 更新（共通処理 + 派生クラスの更新）
   virtual void Update() override;

   /// @brief 描画処理（共通処理 + 派生クラスの描画）
   virtual void Draw() override;

   /// @brief 解放（共通処理 + 派生クラスの解放）
   virtual void Finalize() override;

protected:
   /// @brief リリースカメラの初期設定をカスタマイズ（派生クラスでオーバーライド可能）
   /// @param camera リリースカメラのポインタ
   virtual void SetupReleaseCameraParameters(Camera* camera);

private:

   /// @brief カメラのセットアップ
   void SetupCamera();

   /// @brief ライトのセットアップ
   void SetupLight();

   /// @brief ゲームオブジェクトの更新（派生クラスから呼び出し可能）
   void UpdateGameObjects();

   /// @brief ゲームオブジェクトのImGuiデバッグUI表示
   void DrawGameObjectsImGui();

   /// @brief デバッグ描画を行う（派生クラスでオーバーライド可能）
   virtual void DrawDebug();

   /// @brief 再帰的に描画可能オブジェクトをRenderManagerに追加
   /// @param renderManager レンダーマネージャー
   /// @param drawable 描画可能オブジェクト
   void AddDrawableRecursive(RenderManager* renderManager, IDrawable* drawable);

   /// @brief 削除可能なオブジェクトを削除（描画後に呼ばれる）
   void CleanupGameObjects();

protected:
   // 派生クラスからアクセス可能な共通メンバー
   EngineSystem* engine_ = nullptr;
   std::unique_ptr<CameraManager> cameraManager_;
   DirectionalLightData* directionalLight_ = nullptr;

   // ゲームオブジェクト管理
   std::vector<std::unique_ptr<IDrawable>> gameObjects_;

   // ラインレンダラー（エミッターのデバッグ表示用
   LineRenderer* lineRenderer_ = nullptr;
};
