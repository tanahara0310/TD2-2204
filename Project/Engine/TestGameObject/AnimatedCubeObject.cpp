#include "AnimatedCubeObject.h"
#include <EngineSystem.h>
#include "Engine/Graphics/Model/Animation/Animator.h"
#include <imgui.h>

void AnimatedCubeObject::Initialize() {
   // 必須コンポーネントの取得
   auto engine = GetEngineSystem();

   auto dxCommon = engine->GetComponent<DirectXCommon>();
   auto modelManager = engine->GetComponent<ModelManager>();

   if (!dxCommon || !modelManager) {
	  return;
   }

   // アニメーションを事前に読み込む
   AnimationLoadInfo animLoadInfo{
	   .directory = "Resources/SampleResources/AnimatedCube",
	   .modelFilename = "AnimatedCube.gltf",
	   .animationName = "default"
   };
   modelManager->LoadAnimation(animLoadInfo);

   // キーフレームアニメーションモデルとして作成
   model_ = modelManager->CreateKeyframeModel(
	  "Resources/SampleResources/AnimatedCube/AnimatedCube.gltf",
	  "default",  // アニメーション名
	  true    // ループ再生
   );

   // トランスフォームの初期化
   transform_.Initialize(dxCommon->GetDevice());
   transform_.translate = { 5.0f, 0.0f, 0.0f };  // 他のオブジェクトと被らない位置
   transform_.scale = { 1.0f, 1.0f, 1.0f };
   transform_.rotate = { 0.0f, 0.0f, 0.0f };

   // テクスチャの読み込み
   auto& textureManager = TextureManager::GetInstance();
   texture_ = textureManager.Load("Resources/SampleResources/AnimatedCube/AnimatedCube_BaseColor.png");
}

void AnimatedCubeObject::Update() {
   // アニメーションの更新
   if (model_ && model_->HasAnimationController()) {
	  model_->UpdateAnimation(deltaTime_);
   }

   // トランスフォームの更新
   transform_.TransferMatrix();
}

bool AnimatedCubeObject::DrawImGui() {
   bool changed = false;

   // オブジェクト名をCollapsingHeaderとして表示（タブ）
   if (ImGui::CollapsingHeader(GetObjectName())) {
	  ImGui::PushID(GetObjectName());

	  // トランスフォーム制御（TreeNode）
	  if (ImGui::TreeNode("トランスフォーム")) {
		 Vector3& scale = transform_.scale;
		 Vector3& rotate = transform_.rotate;
		 Vector3& translate = transform_.translate;

		 if (ImGui::DragFloat3("スケール", &scale.x, 0.01f)) {
			changed = true;
		 }

		 if (ImGui::DragFloat3("回転", &rotate.x, 0.01f)) {
			changed = true;
		 }

		 if (ImGui::DragFloat3("位置", &translate.x, 0.1f)) {
			changed = true;
		 }

		 if (ImGui::Button("トランスフォームをリセット")) {
			transform_.scale = { 1.0f, 1.0f, 1.0f };
			transform_.rotate = { 0.0f, 0.0f, 0.0f };
			changed = true;
		 }

		 ImGui::TreePop();
	  }

	  // マテリアル制御（TreeNode）
	  if (model_) {
		 MaterialManager* mat = model_->GetMaterialManager();
		 if (mat && ImGui::TreeNode("マテリアル")) {
			Vector4& colorVec = mat->GetMaterialData()->color;
			float col[4] = { colorVec.x, colorVec.y, colorVec.z, colorVec.w };
			if (ImGui::ColorEdit4("色", col)) {
			   mat->SetColor({ col[0], col[1], col[2], col[3] });
			   changed = true;
			}

			static const char* shadingItems[] = { "なし", "ランバート", "ハーフランバート", "トゥーン" };
			int currentShadingMode = mat->GetMaterialData()->shadingMode;
			if (ImGui::Combo("シェーディングモード", &currentShadingMode, shadingItems, IM_ARRAYSIZE(shadingItems))) {
			   mat->GetMaterialData()->shadingMode = currentShadingMode;
			   changed = true;
			}

			ImGui::TreePop();
		 }
	  }

	  // アニメーション制御（TreeNode）
	  if (model_ && model_->HasAnimationController()) {
		 if (ImGui::TreeNode("アニメーション")) {
			// 再生速度
			float animSpeed = GetAnimationSpeed();
			if (ImGui::SliderFloat("速度", &animSpeed, 0.0f, 3.0f)) {
			   SetAnimationSpeed(animSpeed);
			   changed = true;
			}

			// アニメーション時刻表示
			float animTime = GetAnimationTime();
			ImGui::Text("時刻: %.2f 秒", animTime);

			// リセットボタン
			if (ImGui::Button("アニメーションをリセット")) {
			   ResetAnimation();
			   changed = true;
			}

			ImGui::TreePop();
		 }
	  }

	  ImGui::PopID();
   }

   return changed;
}

void AnimatedCubeObject::SetAnimationSpeed(float /* speed */) {
   // 再生速度の変更はまだ実装されていない
   // TODO: Animatorに速度変更機能を追加する
}

float AnimatedCubeObject::GetAnimationSpeed() const {
   // 再生速度の取得はまだ実装されていない
   return 1.0f;
}

void AnimatedCubeObject::ResetAnimation() {
   if (model_) {
	  model_->ResetAnimation();
   }
}

float AnimatedCubeObject::GetAnimationTime() const {
   if (model_) {
	  return model_->GetAnimationTime();
   }
   return 0.0f;
}

bool AnimatedCubeObject::IsAnimationFinished() const {
   if (model_) {
	  return model_->IsAnimationFinished();
   }
   return true;
}
