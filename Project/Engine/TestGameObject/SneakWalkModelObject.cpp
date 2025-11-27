#include "SneakWalkModelObject.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Utility/FrameRate/FrameRateController.h"
#include "Engine/Graphics/Model/Skeleton/SkeletonDebugRenderer.h"
#include "Engine/Graphics/Material/MaterialManager.h"
#include <imgui.h>

void SneakWalkModelObject::Initialize() {
   auto engine = GetEngineSystem();

	  // ModelManagerを取得
	  auto modelManager = engine->GetComponent<ModelManager>();
   if (!modelManager) {
	  return;
   }

   // アニメーションを事前に読み込む
   AnimationLoadInfo animInfo;
   animInfo.directory = "Resources/SampleResources/human";
   animInfo.modelFilename = "sneakWalk.gltf";
   animInfo.animationName = "sneakWalkAnimation";
   animInfo.animationFilename = "sneakWalk.gltf";
   modelManager->LoadAnimation(animInfo);

   // スケルトンアニメーションモデルとして作成
   model_ = modelManager->CreateSkeletonModel(
	  "Resources/SampleResources/human/sneakWalk.gltf",
	  "sneakWalkAnimation",
	  true
   );

   // Transformの初期化
   auto dxCommon = engine->GetComponent<DirectXCommon>();
   if (dxCommon) {
	  transform_.Initialize(dxCommon->GetDevice());
   }

   // 初期位置・スケール設定（中央に配置）
   transform_.translate = { 0.0f, 0.0f, 0.0f };
   transform_.scale = { 1.0f, 1.0f, 1.0f };
   transform_.rotate = { 0.0f, 0.0f, 0.0f };

   // Joint半径を設定
   jointRadius_ = 0.05f;

   // テクスチャを初期化時に読み込む
   uvCheckerTexture_ = TextureManager::GetInstance().Load("Resources/SampleResources/uvChecker.png");

   // アクティブ状態に設定
   SetActive(true);
}

void SneakWalkModelObject::Update() {
   if (!IsActive() || !model_) {
	  return;
   }

   auto engine = GetEngineSystem();

   // Transformの更新
   transform_.TransferMatrix();

   // FrameRateControllerから1フレームあたりの時間を取得
   auto frameRateController = engine->GetComponent<FrameRateController>();
   if (!frameRateController) {
	  return;
   }

   float deltaTime = frameRateController->GetDeltaTime();

   // アニメーションの更新（コントローラー経由で自動的にスケルトンも更新される）
   if (model_->HasAnimationController()) {
	  model_->UpdateAnimation(deltaTime);
   }
}

void SneakWalkModelObject::Draw(const ICamera* camera) {
   if (!model_ || !camera) return;

   // モデルの描画（ライトは自動セット）
   model_->Draw(transform_, camera, uvCheckerTexture_.gpuHandle);
}

void SneakWalkModelObject::DrawDebug(std::vector<LineRenderer::Line>& outLines) {
   if (!drawSkeleton_ || !model_ || !model_->GetSkeleton()) {
	  return;
   }

   // スケルトンのラインを生成
   SkeletonDebugRenderer::GenerateSkeletonLines(
	  *model_->GetSkeleton(),
	  transform_.GetWorldMatrix(),
	  jointRadius_,
	  outLines
   );
}

bool SneakWalkModelObject::DrawImGuiExtended() {
   bool changed = false;

   // Skeleton制御（特殊機能）
   if (model_) {
      const auto& skeletonOpt = model_->GetSkeleton();
      if (skeletonOpt.has_value()) {
         changed |= SkeletonDebugRenderer::DrawSkeletonImGui(
            &skeletonOpt.value(),
            drawSkeleton_,
            jointRadius_,
            GetObjectName()
         );
      }
   }

   return changed;
}
