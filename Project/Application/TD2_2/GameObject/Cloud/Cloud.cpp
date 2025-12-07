#include "Cloud.h"
#include "../Engine/Utility/Random/RandomGenerator.h"

void Cloud::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	float randomPosX = RangeFloat(-50.0f, -25.0f);
	float randomPosY = RangeFloat(-15.0f, 5.0f);
	float randomPosZ = RangeFloat(50.0f, 70.0f);

	transform_.translate = {randomPosX, randomPosY, randomPosZ};

	// 進行速度
	velocityX_ = RangeFloat(0.05f, 0.1f);

	transform_.TransferMatrix();
}

void Cloud::Update() { 
	// 進行方向に移動
    transform_.translate.x += velocityX_;

	// 座標リセット
	if (transform_.translate.x >= kEndLineX) {
		transform_.translate.x = RangeFloat(-50.0f, -25.0f);
		velocityX_ = RangeFloat(0.1f, 0.3f); // 速度を再設定
	}

	transform_.TransferMatrix(); 
}

void Cloud::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

float Cloud::RangeFloat(float min, float max) {
	static std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}