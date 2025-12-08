#include "CinematicPresetManager.h"
#include "Engine/Utility/JsonManager/JsonManager.h"

CinematicPresetManager& CinematicPresetManager::GetInstance()
{
	static CinematicPresetManager instance;
	return instance;
}

void CinematicPresetManager::RegisterPreset(const std::string& name, const CameraController::CinematicConfig& config)
{
	presets_[name] = config;
}

const CameraController::CinematicConfig* CinematicPresetManager::GetPreset(const std::string& name) const
{
	auto it = presets_.find(name);
	if (it != presets_.end()) {
		return &it->second;
	}
	return nullptr;
}

void CinematicPresetManager::RegisterSequence(const std::string& name, std::shared_ptr<CinematicSequence> sequence)
{
	sequences_[name] = sequence;
}

std::shared_ptr<CinematicSequence> CinematicPresetManager::GetSequence(const std::string& name) const
{
	auto it = sequences_.find(name);
	if (it != sequences_.end()) {
		return it->second;
	}
	return nullptr;
}

bool CinematicPresetManager::LoadPresetsFromJson(const std::string& jsonPath)
{
	try {
		auto& jsonManager = JsonManager::GetInstance();
		json presetsData = jsonManager.LoadJson(jsonPath);

		if (!presetsData.contains("presets") || !presetsData["presets"].is_object()) {
			return false;
		}

		for (auto& [name, presetData] : presetsData["presets"].items()) {
			CameraController::CinematicConfig config;

			// タイプの読み込み
			std::string typeStr = JsonManager::SafeGet<std::string>(presetData, "type", "Dolly");
			if (typeStr == "FixedPosition") {
				config.type = CameraController::CinematicType::FixedPosition;
			} else if (typeStr == "LookAt") {
				config.type = CameraController::CinematicType::LookAt;
			} else if (typeStr == "Dolly") {
				config.type = CameraController::CinematicType::Dolly;
			} else if (typeStr == "Arc") {
				config.type = CameraController::CinematicType::Arc;
			} else if (typeStr == "Orbit") {
				config.type = CameraController::CinematicType::Orbit;
			}

			config.duration = JsonManager::SafeGet<float>(presetData, "duration", 3.0f);
			config.startPosition = JsonManager::SafeGetVector3(presetData, "startPosition", {0, 0, 0});
			config.endPosition = JsonManager::SafeGetVector3(presetData, "endPosition", {0, 0, 0});
			config.targetPosition = JsonManager::SafeGetVector3(presetData, "targetPosition", {0, 0, 0});
			config.startRotation = JsonManager::SafeGetVector3(presetData, "startRotation", {0, 0, 0});
			config.endRotation = JsonManager::SafeGetVector3(presetData, "endRotation", {0, 0, 0});
			config.orbitRadius = JsonManager::SafeGet<float>(presetData, "orbitRadius", 10.0f);
			config.orbitSpeed = JsonManager::SafeGet<float>(presetData, "orbitSpeed", 1.0f);
			config.useEasing = JsonManager::SafeGet<bool>(presetData, "useEasing", true);
			config.easingType = JsonManager::SafeGet<std::string>(presetData, "easingType", "EaseInOutQuad");

			RegisterPreset(name, config);
		}

		return true;
	}
	catch (...) {
		return false;
	}
}

bool CinematicPresetManager::LoadSequenceFromJson(const std::string& name, const std::string& jsonPath)
{
	auto sequence = std::make_shared<CinematicSequence>();
	if (sequence->LoadFromJson(jsonPath)) {
		RegisterSequence(name, sequence);
		return true;
	}
	return false;
}

void CinematicPresetManager::RegisterDefaultPresets()
{
	// オープニング演出：遠くから近づく
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::Dolly;
		config.duration = 0.75f;
		config.startPosition = {0.0f, 0.0f, -80.0f};
		config.endPosition = {0.0f, 0.0f, -50.0f};
		config.startRotation = {0.0f, 0.0f, 0.0f};
		config.endRotation = {0.0f, 0.0f, 0.0f};
		config.useEasing = true;
		config.easingType = "EaseInOutQuad";
		RegisterPreset("Opening", config);
	}

	// ボス登場演出：ボスを中心に回転
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::Orbit;
		config.duration = 4.0f;
		config.targetPosition = {0.0f, 0.0f, 0.0f};
		config.startPosition = {0.0f, 5.0f, 0.0f};
		config.orbitRadius = 25.0f;
		config.orbitSpeed = 1.0f;
		config.useEasing = true;
		config.easingType = "EaseInOutQuad";
		RegisterPreset("BossAppear", config);
	}

	// 勝利演出：プレイヤーを注視
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::LookAt;
		config.duration = 3.0f;
		config.startPosition = {-10.0f, 5.0f, -20.0f};
		config.targetPosition = {0.0f, 0.0f, 0.0f};
		config.useEasing = true;
		config.easingType = "EaseOutQuad";
		RegisterPreset("Victory", config);
	}

	// 敗北演出：プレイヤーの上から見下ろす
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::Dolly;
		config.duration = 3.0f;
		config.startPosition = {0.0f, 5.0f, -20.0f};
		config.endPosition = {0.0f, 15.0f, -10.0f};
		config.startRotation = {0.0f, 0.0f, 0.0f};
		config.endRotation = {0.5f, 0.0f, 0.0f};
		config.useEasing = true;
		config.easingType = "EaseInQuad";
		RegisterPreset("Defeat", config);
	}

	// クローズアップ：キャラクターに近づく
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::Dolly;
		config.duration = 2.0f;
		config.startPosition = {0.0f, 2.0f, -15.0f};
		config.endPosition = {0.0f, 1.0f, -5.0f};
		config.startRotation = {0.0f, 0.0f, 0.0f};
		config.endRotation = {0.1f, 0.0f, 0.0f};
		config.useEasing = true;
		config.easingType = "EaseInOutCubic";
		RegisterPreset("CloseUp", config);
	}

	// 俯瞰：ステージ全体を見下ろす
	{
		CameraController::CinematicConfig config;
		config.type = CameraController::CinematicType::FixedPosition;
		config.duration = 3.0f;
		config.startPosition = {0.0f, 50.0f, 0.0f};
		config.startRotation = {1.57f, 0.0f, 0.0f}; // 90度下向き
		config.useEasing = true;
		config.easingType = "Linear";
		RegisterPreset("BirdsEye", config);
	}
}

std::vector<std::string> CinematicPresetManager::GetPresetNames() const
{
	std::vector<std::string> names;
	names.reserve(presets_.size());
	for (const auto& [name, _] : presets_) {
		names.push_back(name);
	}
	return names;
}

std::vector<std::string> CinematicPresetManager::GetSequenceNames() const
{
	std::vector<std::string> names;
	names.reserve(sequences_.size());
	for (const auto& [name, _] : sequences_) {
		names.push_back(name);
	}
	return names;
}

bool CinematicPresetManager::HasPreset(const std::string& name) const
{
	return presets_.find(name) != presets_.end();
}

bool CinematicPresetManager::HasSequence(const std::string& name) const
{
	return sequences_.find(name) != sequences_.end();
}
