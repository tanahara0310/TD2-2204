#include "TitleLightningFrameManager.h"
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "IDrawable.h"

void TitleLightningFrameManager::Initialize(LightningEffectManager* lightningManager, std::vector<std::unique_ptr<IDrawable>>& gameObjects) {
	lightningManager_ = lightningManager;

	if (!lightningManager_) {
		return;
	}

	LightningEffectManager::EffectConfig config;
	config.segmentCount = 4;
	config.noiseScale = 0.8f;
	config.noiseSpeed = 60.0f;
	config.randomOffsetRange = 0.3f;
	config.voxelScale = { 1.0f, 1.0f, 1.0f };
	config.initialVisible = false;
	config.voxelSpacing = 0.2f;

	// 矩形サイズ
	float halfWidth = 1.35f;
	float halfHeight = 0.58f;

	// 初期中心はStartの位置
	Vector3 startCenter = kStartCenter_;

	config.color = kStartColor_;

	// 上辺（index 0）
	config.startOffset = { -halfWidth,  halfHeight, 0.0f };
	config.endOffset = { halfWidth,  halfHeight, 0.0f };
	frameEffectIds_[0] = lightningManager_->CreateEffect(startCenter, config, gameObjects);

	// 下辺（index 1）
	config.startOffset = { -halfWidth, -halfHeight, 0.0f };
	config.endOffset = { halfWidth, -halfHeight, 0.0f };
	frameEffectIds_[1] = lightningManager_->CreateEffect(startCenter, config, gameObjects);

	// 左辺（index 2）
	config.startOffset = { -halfWidth, -halfHeight, 0.0f };
	config.endOffset = { -halfWidth,  halfHeight, 0.0f };
	frameEffectIds_[2] = lightningManager_->CreateEffect(startCenter, config, gameObjects);

	// 右辺（index 3）
	config.startOffset = { halfWidth, -halfHeight, 0.0f };
	config.endOffset = { halfWidth,  halfHeight, 0.0f };
	frameEffectIds_[3] = lightningManager_->CreateEffect(startCenter, config, gameObjects);

	// 初期は非表示
	for (int i = 0; i < 4; ++i) {
		if (frameEffectIds_[i] >= 0) {
			lightningManager_->SetEffectVisible(frameEffectIds_[i], false);
		}
	}
}

void TitleLightningFrameManager::Update(float deltaTime, bool isConfirmAnimating) {
	if (!lightningManager_) {
		return;
	}

	// 雷エフェクトの更新
	lightningManager_->UpdateAllEffects();

	// 決定演出中はパルス演出をスキップ
	if (!isConfirmAnimating) {
		UpdatePulseEffect(deltaTime);
	}
}

void TitleLightningFrameManager::UpdateFramePosition(bool isStartSelected) {
	if (!lightningManager_) {
		return;
	}

	Vector3 target = isStartSelected ? kStartCenter_ : kQuitCenter_;

	// 枠の位置を更新
	for (int i = 0; i < 4; i++) {
		if (frameEffectIds_[i] >= 0) {
			lightningManager_->SetEffectPosition(frameEffectIds_[i], target);
		}
	}

	// 選択状態が変わった場合のみ色を更新
	if (lastIsStartSelected_ != isStartSelected) {
		Vector4 color = isStartSelected ? kStartColor_ : kQuitColor_;
		for (int i = 0; i < 4; i++) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectColor(frameEffectIds_[i], color);
			}
		}
		lastIsStartSelected_ = isStartSelected;
	}
}

void TitleLightningFrameManager::UpdatePulseEffect(float deltaTime) {
	if (!lightningManager_) {
		return;
	}

	pulseTimer_ += deltaTime;

	// パルス未表示状態で、間隔到達したら開始
	if (visibleTimer_ <= 0.0f && pulseTimer_ >= nextPulseInterval_) {
		pulseTimer_ = 0.0f;
		visibleTimer_ = kPulseDuration_;
		flickerTimer_ = 0.0f;

		// 次回間隔を乱数で設定
		nextPulseInterval_ = RandomGenerator::GetInstance().GetFloat(kPulseIntervalMin_, kPulseIntervalMax_);

		// 辺をランダム選択（2本、重複なし）
		currentEdgeIndexA_ = RandomGenerator::GetInstance().GetInt(0, 3);
		do {
			currentEdgeIndexB_ = RandomGenerator::GetInstance().GetInt(0, 3);
		} while (currentEdgeIndexB_ == currentEdgeIndexA_);

		// 選ばれた2辺のみ即座に表示、その他は即座に非表示
		for (int i = 0; i < 4; i++) {
			if (frameEffectIds_[i] < 0) continue;
			bool isSelected = (i == currentEdgeIndexA_ || i == currentEdgeIndexB_);
			lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], isSelected);
		}
	}

	// 表示中なら雷っぽく点滅
	if (visibleTimer_ > 0.0f && currentEdgeIndexA_ >= 0 && currentEdgeIndexB_ >= 0) {
		visibleTimer_ -= deltaTime;
		flickerTimer_ += deltaTime;
		if (flickerTimer_ >= kFlickerInterval_) {
			flickerTimer_ = 0.0f;
			static bool flickerOn = false;
			flickerOn = !flickerOn;
			lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[currentEdgeIndexA_], flickerOn);
			lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[currentEdgeIndexB_], flickerOn);

			// 選ばれていない辺は常に非表示
			for (int i = 0; i < 4; i++) {
				if ((i == currentEdgeIndexA_ || i == currentEdgeIndexB_) || frameEffectIds_[i] < 0) continue;
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
			}
		}

		// 表示時間終了で全て非表示
		if (visibleTimer_ <= 0.0f) {
			for (int i = 0; i < 4; ++i) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
				}
			}
			currentEdgeIndexA_ = -1;
			currentEdgeIndexB_ = -1;
		}
	}
}

void TitleLightningFrameManager::ShowAllEdges() {
	if (!lightningManager_) {
		return;
	}

	for (int i = 0; i < 4; ++i) {
		if (frameEffectIds_[i] >= 0) {
			lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], true);
		}
	}
}

void TitleLightningFrameManager::HideAllEdges() {
	if (!lightningManager_) {
		return;
	}

	for (int i = 0; i < 4; ++i) {
		if (frameEffectIds_[i] >= 0) {
			lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
		}
	}
}
