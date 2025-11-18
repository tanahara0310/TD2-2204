#include "SneakWalkModelObject.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Utility/FrameRate/FrameRateController.h"
#include "Engine/Graphics/Model/Skeleton/SkeletonDebugRenderer.h"
#include "Engine/Graphics/Material/MaterialManager.h"
#include <imgui.h>

void SneakWalkModelObject::Initialize(EngineSystem* engine) {
	engine_ = engine;

	// ModelManagerを取得
	auto modelManager = engine_->GetComponent<ModelManager>();
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
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
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

	// Transformの更新
	transform_.TransferMatrix();

	// FrameRateControllerから1フレームあたりの時間を取得
	auto frameRateController = engine_->GetComponent<FrameRateController>();
	if (!frameRateController) {
		return;
	}
	
	float deltaTime = frameRateController->GetDeltaTime();

	// アニメーションの更新（コントローラー経由で自動的にスケルトンも更新される）
	if (model_->HasAnimationController()) {
		model_->UpdateAnimation(deltaTime);
	}
}

void SneakWalkModelObject::Draw(ICamera* camera) {
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

bool SneakWalkModelObject::DrawImGui() {
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
		
		// Skeleton制御（共通実装を使用）
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
		
		ImGui::PopID();
	}
	
	return changed;
}
