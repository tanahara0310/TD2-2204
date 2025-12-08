#include "ControllerModel.h"
#include "../../Utility/GameUtils.h"

void ControllerModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);
	
	// 初期位置とスケールの設定
	transform_.translate = { 0.0f, 0.0f, 10.0f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.TransferMatrix();
	
	// 初期アルファ値を設定
	blinkTimer_ = 0.0f;
	isHighAlpha_ = true;
}

void ControllerModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// 点滅アニメーションの更新
	UpdateBlinkAnimation(deltaTime);
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void ControllerModel::UpdateBlinkAnimation(float deltaTime) {
	blinkTimer_ += deltaTime;
	
	// 点滅間隔に達したら切り替え
	if (blinkTimer_ >= kBlinkInterval) {
		blinkTimer_ = 0.0f;
		isHighAlpha_ = !isHighAlpha_;
	}
}

void ControllerModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}
	
	// マテリアルにアルファ値を設定
	if (auto* materialManager = model_->GetMaterialManager()) {
		Vector4 color = materialManager->GetColor();
		color.w = isHighAlpha_ ? kAlphaHigh : kAlphaLow;
		materialManager->SetColor(color);
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
