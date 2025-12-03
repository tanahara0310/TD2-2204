#include "LightningEffectManager.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <numbers>

#ifdef _DEBUG
#include <imgui.h>
#endif

void LightningEffectManager::Initialize(ModelManager* modelManager, TextureManager* textureManager)
{
	// 共有リソースの読み込み
	modelManager->LoadModelResource("Resources/Models/Voxel/", "voxel.obj");
	voxelModelResource_ = modelManager->GetModelResource("Resources/Models/Voxel/voxel.obj");
	voxelTexture_ = textureManager->Load("Resources/SampleResources/white1x1.png");
}

int LightningEffectManager::CreateCircularEffect(GameObject* target, const CircularEffectConfig& config,
	std::vector<std::unique_ptr<IDrawable>>& gameObjects)
{
	if (!target || !voxelModelResource_) {
		return -1;
	}

	// エフェクトデータを作成
	EffectData effectData;
	effectData.target = target;
	effectData.config = config;

	const float angleStep = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(config.lightningCount);
	effectData.visibility.resize(config.lightningCount, false);

	// 雷を生成
	for (int i = 0; i < config.lightningCount; ++i) {
		float baseAngle = angleStep * i;
		float startAngle = baseAngle;
		float endAngle = baseAngle + config.arcLength;

		Vector3 startPos = {
			std::cos(startAngle) * config.radius,
			std::sin(startAngle) * config.radius,
			0.0f
		};

		Vector3 endPos = {
			std::cos(endAngle) * config.radius,
			std::sin(endAngle) * config.radius,
			0.0f
		};

		// 雷の設定
		Lightning::Config lightningConfig;
		lightningConfig.startPoint = startPos;
		lightningConfig.endPoint = endPos;
		lightningConfig.segmentCount = config.segmentCount;
		lightningConfig.noiseScale = config.noiseScale;
		lightningConfig.noiseSpeed = config.noiseSpeed;
		lightningConfig.enableAnimation = true;
		lightningConfig.color = config.color;
		lightningConfig.pathType = Lightning::PathType::CircularArc;

		auto lightning = std::make_unique<Lightning>();
		lightning->Initialize(voxelModelResource_, voxelTexture_, lightningConfig,
			"Lightning_" + std::to_string(nextEffectId_) + "_" + std::to_string(i));
		lightning->SetActive(false);

		effectData.lightnings.push_back(lightning.get());
		gameObjects.push_back(std::move(lightning));
	}

	int effectId = nextEffectId_++;
	effects_.push_back(std::move(effectData));
	return effectId;
}

void LightningEffectManager::UpdateAllEffects()
{
	for (auto& effect : effects_) {
		if (!effect.target) continue;

		// 位置を更新
		UpdateEffectPosition(effect);

		// タイマーを更新
		if (effect.isActive) {
			UpdateEffectTimer(effect);
		}
	}
}

void LightningEffectManager::StartEffect(int effectId)
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}

	EffectData& effect = effects_[effectId];
	effect.isActive = true;
	effect.timer = 0.0f;

	const int lightningCount = static_cast<int>(effect.lightnings.size());

	// ランダムに雷を表示
	for (int i = 0; i < lightningCount; ++i) {
		float randomValue = GameUtils::RandomFloat(0.0f, 1.0f);
		bool shouldShow = randomValue < effect.config.displayProbability;

		effect.visibility[i] = shouldShow;
		if (effect.lightnings[i]) {
			effect.lightnings[i]->SetActive(shouldShow);
		}
	}

	// 最低表示数を保証
	int visibleCount = 0;
	for (bool visible : effect.visibility) {
		if (visible) visibleCount++;
	}

	while (visibleCount < effect.config.minVisibleCount && lightningCount > 0) {
		int randomIndex = static_cast<int>(GameUtils::RandomFloat(0.0f,
			static_cast<float>(lightningCount - 1) + 0.999f));

		if (!effect.visibility[randomIndex]) {
			effect.visibility[randomIndex] = true;
			if (effect.lightnings[randomIndex]) {
				effect.lightnings[randomIndex]->SetActive(true);
			}
			visibleCount++;
		}

		if (visibleCount >= lightningCount) break;
	}
}

void LightningEffectManager::StopEffect(int effectId)
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}

	EffectData& effect = effects_[effectId];
	effect.isActive = false;
	effect.timer = 0.0f;

	// 全ての雷を非表示
	for (size_t i = 0; i < effect.lightnings.size(); ++i) {
		effect.visibility[i] = false;
		if (effect.lightnings[i]) {
			effect.lightnings[i]->SetActive(false);
		}
	}
}

bool LightningEffectManager::IsEffectActive(int effectId) const
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return false;
	}
	return effects_[effectId].isActive;
}

LightningEffectManager::CircularEffectConfig& LightningEffectManager::GetEffectConfig(int effectId)
{
	static CircularEffectConfig dummy;
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return dummy;
	}
	return effects_[effectId].config;
}

void LightningEffectManager::DrawDebugUI(int effectId, const char* windowName)
{
#ifdef _DEBUG
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return;
	}

	EffectData& effect = effects_[effectId];

	if (ImGui::Begin(windowName)) {
		if (ImGui::Button("Test Effect")) {
			StartEffect(effectId);
		}

		ImGui::Text("Active: %s", effect.isActive ? "Yes" : "No");
		ImGui::Text("Effect ID: %d", effectId);

		ImGui::Separator();

		ImGui::DragFloat("Radius", &effect.config.radius, 0.1f, 0.5f, 10.0f);

		if (ImGui::ColorEdit4("Color", &effect.config.color.x)) {
			UpdateLightningColors(effect);
		}

		ImGui::DragFloat("Duration", &effect.config.effectDuration, 0.01f, 0.1f, 2.0f);
		ImGui::DragFloat("Noise Scale", &effect.config.noiseScale, 0.01f, 0.0f, 2.0f);
		ImGui::DragFloat("Noise Speed", &effect.config.noiseSpeed, 0.1f, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Lightnings: %zu", effect.lightnings.size());
		ImGui::Text("Timer: %.2f / %.2f", effect.timer, effect.config.effectDuration);
	}
	ImGui::End();
#endif
}

void LightningEffectManager::UpdateEffectPosition(EffectData& effect)
{
	if (!effect.target || effect.lightnings.empty()) {
		return;
	}

	Vector3 targetPos = effect.target->GetWorldPosition();
	const float angleStep = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(effect.lightnings.size());

	for (size_t i = 0; i < effect.lightnings.size(); ++i) {
		auto* lightning = effect.lightnings[i];
		if (!lightning) continue;

		// 位置を更新
		lightning->GetTransform().translate = targetPos;

		// 始点・終点を更新
		float baseAngle = angleStep * i;
		float startAngle = baseAngle;
		float endAngle = baseAngle + effect.config.arcLength;

		auto& config = lightning->GetConfig();
		config.startPoint = {
			std::cos(startAngle) * effect.config.radius,
			std::sin(startAngle) * effect.config.radius,
			0.0f
		};
		config.endPoint = {
			std::cos(endAngle) * effect.config.radius,
			std::sin(endAngle) * effect.config.radius,
			0.0f
		};
	}
}

void LightningEffectManager::UpdateEffectTimer(EffectData& effect)
{
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	effect.timer += deltaTime;

	if (effect.timer >= effect.config.effectDuration) {
		effect.isActive = false;
		effect.timer = 0.0f;

		// 全ての雷を非表示
		for (size_t i = 0; i < effect.lightnings.size(); ++i) {
			effect.visibility[i] = false;
			if (effect.lightnings[i]) {
				effect.lightnings[i]->SetActive(false);
			}
		}
	}
}

void LightningEffectManager::UpdateLightningColors(EffectData& effect)
{
	for (auto* lightning : effect.lightnings) {
		if (lightning) {
			lightning->GetConfig().color = effect.config.color;
		}
	}
}
