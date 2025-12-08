#include "CinematicSequence.h"
#include "Engine/Utility/JsonManager/JsonManager.h"

void CinematicSequence::AddCut(const CinematicCut& cut)
{
	cuts_.push_back(cut);
	totalDuration_ += cut.duration;
}

void CinematicSequence::Start()
{
	if (cuts_.empty()) {
		return;
	}

	isActive_ = true;
	currentCutIndex_ = 0;
	elapsedTime_ = 0.0f;

	// 最初のカットのタイマーを開始
	if (currentCutIndex_ < static_cast<int>(cuts_.size())) {
		cutTimer_.Start(cuts_[currentCutIndex_].duration, false);
	}
}

void CinematicSequence::Stop()
{
	isActive_ = false;
	currentCutIndex_ = -1;
	cutTimer_.Stop();
	elapsedTime_ = 0.0f;
}

void CinematicSequence::Update(float deltaTime)
{
	if (!isActive_ || currentCutIndex_ < 0 || currentCutIndex_ >= static_cast<int>(cuts_.size())) {
		return;
	}

	elapsedTime_ += deltaTime;
	cutTimer_.Update(deltaTime);

	// 現在のカットが終了したら次のカットへ
	if (cutTimer_.IsFinished()) {
		currentCutIndex_++;

		// 全てのカットが終了したらシーケンスを停止
		if (currentCutIndex_ >= static_cast<int>(cuts_.size())) {
			Stop();
			return;
		}

		// 次のカットのタイマーを開始
		cutTimer_.Start(cuts_[currentCutIndex_].duration, false);
	}
}

float CinematicSequence::GetProgress() const
{
	if (totalDuration_ <= 0.0f) {
		return 0.0f;
	}
	return elapsedTime_ / totalDuration_;
}

const CinematicCut* CinematicSequence::GetCurrentCut() const
{
	if (currentCutIndex_ < 0 || currentCutIndex_ >= static_cast<int>(cuts_.size())) {
		return nullptr;
	}
	return &cuts_[currentCutIndex_];
}

const CinematicCut* CinematicSequence::GetCutAt(int index) const
{
	if (index < 0 || index >= static_cast<int>(cuts_.size())) {
		return nullptr;
	}
	return &cuts_[index];
}

bool CinematicSequence::LoadFromJson(const std::string& jsonPath)
{
	try {
		auto& jsonManager = JsonManager::GetInstance();
		json sequenceData = jsonManager.LoadJson(jsonPath);

		Clear();

		if (!sequenceData.contains("cuts") || !sequenceData["cuts"].is_array()) {
			return false;
		}

		for (const auto& cutData : sequenceData["cuts"]) {
			CinematicCut cut;

			// カット名
			cut.name = JsonManager::SafeGet<std::string>(cutData, "name", "Cut");

			// 継続時間
			cut.duration = JsonManager::SafeGet<float>(cutData, "duration", 1.0f);

			// タイプの読み込み
			std::string typeStr = JsonManager::SafeGet<std::string>(cutData, "type", "FixedPosition");
			if (typeStr == "FixedPosition") {
				cut.config.type = CameraController::CinematicType::FixedPosition;
			} else if (typeStr == "LookAt") {
				cut.config.type = CameraController::CinematicType::LookAt;
			} else if (typeStr == "Dolly") {
				cut.config.type = CameraController::CinematicType::Dolly;
			} else if (typeStr == "Arc") {
				cut.config.type = CameraController::CinematicType::Arc;
			} else if (typeStr == "Orbit") {
				cut.config.type = CameraController::CinematicType::Orbit;
			}

			// 演出設定
			cut.config.duration = cut.duration;
			cut.config.startPosition = JsonManager::SafeGetVector3(cutData, "startPosition", {0, 0, 0});
			cut.config.endPosition = JsonManager::SafeGetVector3(cutData, "endPosition", {0, 0, 0});
			cut.config.targetPosition = JsonManager::SafeGetVector3(cutData, "targetPosition", {0, 0, 0});
			cut.config.startRotation = JsonManager::SafeGetVector3(cutData, "startRotation", {0, 0, 0});
			cut.config.endRotation = JsonManager::SafeGetVector3(cutData, "endRotation", {0, 0, 0});
			cut.config.orbitRadius = JsonManager::SafeGet<float>(cutData, "orbitRadius", 10.0f);
			cut.config.orbitSpeed = JsonManager::SafeGet<float>(cutData, "orbitSpeed", 1.0f);
			cut.config.useEasing = JsonManager::SafeGet<bool>(cutData, "useEasing", true);
			cut.config.easingType = JsonManager::SafeGet<std::string>(cutData, "easingType", "EaseInOutQuad");

			AddCut(cut);
		}

		return true;
	}
	catch (...) {
		return false;
	}
}

bool CinematicSequence::SaveToJson(const std::string& jsonPath) const
{
	try {
		auto& jsonManager = JsonManager::GetInstance();
		json sequenceData;

		json cutsArray = json::array();

		for (const auto& cut : cuts_) {
			json cutData;

			// カット名
			cutData["name"] = cut.name;

			// 継続時間
			cutData["duration"] = cut.duration;

			// タイプ
			std::string typeStr = "FixedPosition";
			switch (cut.config.type) {
			case CameraController::CinematicType::FixedPosition: typeStr = "FixedPosition"; break;
			case CameraController::CinematicType::LookAt: typeStr = "LookAt"; break;
			case CameraController::CinematicType::Dolly: typeStr = "Dolly"; break;
			case CameraController::CinematicType::Arc: typeStr = "Arc"; break;
			case CameraController::CinematicType::Orbit: typeStr = "Orbit"; break;
			}
			cutData["type"] = typeStr;

			// 演出設定
			cutData["startPosition"] = JsonManager::Vector3ToJson(cut.config.startPosition);
			cutData["endPosition"] = JsonManager::Vector3ToJson(cut.config.endPosition);
			cutData["targetPosition"] = JsonManager::Vector3ToJson(cut.config.targetPosition);
			cutData["startRotation"] = JsonManager::Vector3ToJson(cut.config.startRotation);
			cutData["endRotation"] = JsonManager::Vector3ToJson(cut.config.endRotation);
			cutData["orbitRadius"] = cut.config.orbitRadius;
			cutData["orbitSpeed"] = cut.config.orbitSpeed;
			cutData["useEasing"] = cut.config.useEasing;
			cutData["easingType"] = cut.config.easingType;

			cutsArray.push_back(cutData);
		}

		sequenceData["cuts"] = cutsArray;

		return jsonManager.SaveJson(jsonPath, sequenceData);
	}
	catch (...) {
		return false;
	}
}

void CinematicSequence::Clear()
{
	cuts_.clear();
	currentCutIndex_ = -1;
	totalDuration_ = 0.0f;
	elapsedTime_ = 0.0f;
	isActive_ = false;
	cutTimer_.Stop();
}
