#include "LightningEffectManager.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <numbers>

#ifdef _DEBUG
#include <imgui.h>
#endif

namespace {
	// 黄金角（ゴールデンアングル）
	const float kGoldenAngle = std::numbers::pi_v<float> * (3.0f - std::sqrt(5.0f));
}

void LightningEffectManager::Initialize(ModelManager* modelManager, TextureManager* textureManager)
{
	// 共有リソースの読み込み
	modelManager->LoadModelResource("Resources/Models/Voxel/", "voxel.obj");
	voxelModelResource_ = modelManager->GetModelResource("Resources/Models/Voxel/voxel.obj");
	voxelTexture_ = textureManager->Load("Resources/SampleResources/white1x1.png");
}

std::vector<Vector3> LightningEffectManager::GenerateSphericalPoints(int count, float radius)
{
	std::vector<Vector3> points;
	points.reserve(count);

	// フィボナッチ球面配置アルゴリズム
	// 球面上に均等に点を配置する数学的手法
	for (int i = 0; i < count; ++i) {
		// Y座標（高さ）を計算：-1から1の範囲
		float y = 1.0f - (i / static_cast<float>(count - 1)) * 2.0f;
		
		// Y座標から半径を計算
		float radiusAtY = std::sqrt(1.0f - y * y);
		
		// 黄金角を使用して角度を計算
		float theta = kGoldenAngle * i;
		
		// XZ平面上の座標を計算
		float x = std::cos(theta) * radiusAtY;
		float z = std::sin(theta) * radiusAtY;
		
		// 半径を適用
		points.push_back({ x * radius, y * radius, z * radius });
	}

	return points;
}

int LightningEffectManager::CreateCircularEffect(GameObject* target, const SphericalEffectConfig& config,
	std::vector<std::unique_ptr<IDrawable>>& gameObjects)
{
	if (!target || !voxelModelResource_) {
		return -1;
	}

	// エフェクトデータを作成
	EffectData effectData;
	effectData.target = target;
	effectData.config = config;
	effectData.visibility.resize(config.lightningCount, false);

	// 球面上の点を生成（フィボナッチ球面配置）
	effectData.sphericalPositions = GenerateSphericalPoints(config.lightningCount, config.radius);

	// 雷を生成
	for (int i = 0; i < config.lightningCount; ++i) {
		const Vector3& startPos = effectData.sphericalPositions[i];
		
		// 終点：始点から球の中心方向に少し進んだ位置（円弧を作る）
		Vector3 toCenter = -MathCore::Vector::Normalize(startPos);
		float arcDistance = config.arcLength * config.radius;
		Vector3 endPos = startPos + toCenter * arcDistance;

		// 雷の設定
		Lightning::Config lightningConfig;
		lightningConfig.startPoint = startPos;
		lightningConfig.endPoint = endPos;
		lightningConfig.segmentCount = config.segmentCount;
		lightningConfig.noiseScale = config.noiseScale;
		lightningConfig.noiseSpeed = config.noiseSpeed;
		lightningConfig.enableAnimation = true;
		lightningConfig.color = config.color;
		lightningConfig.pathType = Lightning::PathType::Linear; // 球面では直線補間

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
			// 時間差出現を更新
			UpdateStaggeredSpawns(effect);
			
			// エフェクトタイマーを更新
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
	effect.currentSpawnIndex = 0;

	const int lightningCount = static_cast<int>(effect.lightnings.size());
	const int visibleCount = (effect.config.visibleCount < lightningCount) 
		? effect.config.visibleCount 
		: lightningCount;

	// 全ての雷を非表示にしてからリセット
	SetAllLightningsVisibility(effect, false);

	// ランダムな出現順序を生成
	effect.spawnOrder.clear();
	effect.spawnOrder.reserve(lightningCount);
	for (int i = 0; i < lightningCount; ++i) {
		effect.spawnOrder.push_back(i);
	}

	// Fisher-Yatesシャッフルでランダムに並び替え
	for (int i = lightningCount - 1; i > 0; --i) {
		int j = static_cast<int>(GameUtils::RandomFloat(0.0f, static_cast<float>(i) + 0.999f));
		std::swap(effect.spawnOrder[i], effect.spawnOrder[j]);
	}

	// 出現時刻を計算
	effect.spawnTimes.clear();
	effect.spawnTimes.reserve(visibleCount);
	
	if (effect.config.enableStagger) {
		// 時間差出現が有効な場合
		for (int i = 0; i < visibleCount; ++i) {
			effect.spawnTimes.push_back(i * effect.config.staggerDelay);
		}
	} else {
		// 時間差出現が無効な場合は全て即座に表示
		for (int i = 0; i < visibleCount; ++i) {
			effect.spawnTimes.push_back(0.0f);
			SetLightningVisibility(effect, effect.spawnOrder[i], true);
		}
		effect.currentSpawnIndex = visibleCount;
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
	SetAllLightningsVisibility(effect, false);
}

bool LightningEffectManager::IsEffectActive(int effectId) const
{
	if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
		return false;
	}
	return effects_[effectId].isActive;
}

LightningEffectManager::SphericalEffectConfig& LightningEffectManager::GetEffectConfig(int effectId)
{
	static SphericalEffectConfig dummy;
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

		// 半径変更時は球面位置を再計算
		float oldRadius = effect.config.radius;
		ImGui::DragFloat("Radius", &effect.config.radius, 0.1f, 0.5f, 10.0f);
		if (oldRadius != effect.config.radius) {
			effect.sphericalPositions = GenerateSphericalPoints(
				static_cast<int>(effect.lightnings.size()), 
				effect.config.radius
			);
		}

		if (ImGui::ColorEdit4("Color", &effect.config.color.x)) {
			UpdateLightningColors(effect);
		}

		ImGui::DragFloat("Arc Length", &effect.config.arcLength, 0.01f, 0.1f, 2.0f);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("雷の長さ（半径に対する比率）");
		}

		ImGui::DragInt("Visible Count", &effect.config.visibleCount, 1.0f, 0, effect.config.lightningCount);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("表示する雷の数（常に一定）");
		}

		ImGui::Separator();
		ImGui::Text("Timing Settings");
		
		ImGui::DragFloat("Duration", &effect.config.effectDuration, 0.01f, 0.1f, 2.0f);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("エフェクト全体の継続時間");
		}

		ImGui::Checkbox("Enable Stagger", &effect.config.enableStagger);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("雷を時間差で出現させる");
		}

		if (effect.config.enableStagger) {
			ImGui::DragFloat("Stagger Delay", &effect.config.staggerDelay, 0.01f, 0.0f, 0.5f);
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("各雷の出現間隔（秒）");
			}
		}

		ImGui::Separator();
		ImGui::Text("Visual Settings");
		
		ImGui::DragFloat("Noise Scale", &effect.config.noiseScale, 0.01f, 0.0f, 2.0f);
		ImGui::DragFloat("Noise Speed", &effect.config.noiseSpeed, 0.1f, 0.0f, 30.0f);

		ImGui::Separator();
		ImGui::Text("Debug Info");
		
		ImGui::Text("Total Lightnings: %zu", effect.lightnings.size());
		ImGui::Text("Target Visible: %d", effect.config.visibleCount);
		ImGui::Text("Currently Spawned: %d", effect.currentSpawnIndex);
		ImGui::Text("Timer: %.2f / %.2f", effect.timer, effect.config.effectDuration);
		
		if (effect.config.enableStagger && !effect.spawnTimes.empty()) {
			float totalStaggerTime = effect.spawnTimes.back();
			ImGui::Text("Total Stagger Time: %.3f sec", totalStaggerTime);
		}
	}
	ImGui::End();
#endif
}

void LightningEffectManager::UpdateEffectPosition(EffectData& effect)
{
	if (!effect.target || effect.lightnings.empty()) {
		return;
	}

	const Vector3 targetPos = effect.target->GetWorldPosition();

	for (size_t i = 0; i < effect.lightnings.size(); ++i) {
		auto* lightning = effect.lightnings[i];
		if (!lightning) continue;

		// 位置を更新
		lightning->GetTransform().translate = targetPos;

		// 球面上の始点・終点を更新
		const Vector3& startPos = effect.sphericalPositions[i];
		Vector3 toCenter = -MathCore::Vector::Normalize(startPos);
		float arcDistance = effect.config.arcLength * effect.config.radius;
		Vector3 endPos = startPos + toCenter * arcDistance;

		auto& config = lightning->GetConfig();
		config.startPoint = startPos;
		config.endPoint = endPos;
	}
}

void LightningEffectManager::UpdateStaggeredSpawns(EffectData& effect)
{
	// 時間差出現が無効または全て出現済みの場合は何もしない
	if (!effect.config.enableStagger || 
		effect.currentSpawnIndex >= static_cast<int>(effect.spawnTimes.size())) {
		return;
	}

	// 現在の時刻で出現すべき雷を表示
	while (effect.currentSpawnIndex < static_cast<int>(effect.spawnTimes.size())) {
		float spawnTime = effect.spawnTimes[effect.currentSpawnIndex];
		
		// まだ出現時刻に達していない場合は終了
		if (effect.timer < spawnTime) {
			break;
		}

		// 雷を表示
		int lightningIndex = effect.spawnOrder[effect.currentSpawnIndex];
		SetLightningVisibility(effect, lightningIndex, true);
		
		effect.currentSpawnIndex++;
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
		SetAllLightningsVisibility(effect, false);
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

void LightningEffectManager::SetLightningVisibility(EffectData& effect, int index, bool visible)
{
	if (index < 0 || index >= static_cast<int>(effect.lightnings.size())) {
		return;
	}

	effect.visibility[index] = visible;
	if (effect.lightnings[index]) {
		effect.lightnings[index]->SetActive(visible);
	}
}

void LightningEffectManager::SetAllLightningsVisibility(EffectData& effect, bool visible)
{
	for (size_t i = 0; i < effect.lightnings.size(); ++i) {
		SetLightningVisibility(effect, static_cast<int>(i), visible);
	}
}
