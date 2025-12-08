#include "CameraControllerEditor.h"

#ifdef _DEBUG

#include "CinematicPresetManager.h"
#include "CinematicSequence.h"
#include <imgui.h>

CameraControllerEditor::CameraControllerEditor(CameraController* controller)
	: controller_(controller)
{
}

void CameraControllerEditor::DrawImGui()
{
	if (!controller_) {
		return;
	}

	if (ImGui::Begin("Camera Controller")) {
		DrawCinematicControlSection();
		ImGui::Separator();
		DrawPresetSequenceSection();
		ImGui::Separator();
		DrawSequenceEditorSection();
		ImGui::Separator();
		DrawCameraParametersSection();
		ImGui::Separator();
		DrawStageBoundsSection();
		ImGui::Separator();
		DrawCameraShakeSection();
		ImGui::Separator();
		DrawCurrentStatusSection();
	}
	ImGui::End();
}

void CameraControllerEditor::DrawCinematicControlSection()
{
	if (ImGui::CollapsingHeader("Cinematic Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("状態: %s", controller_->IsCinematicActive() ? "演出中" : "通常追従");

		if (controller_->IsCinematicActive()) {
			float progress = controller_->GetCinematicProgress();
			ImGui::ProgressBar(progress, ImVec2(-1, 0), "進行度");

			if (ImGui::Button("演出停止", ImVec2(120, 0))) {
				controller_->StopCinematic();
			}
		}

		ImGui::Separator();
		DrawCinematicSettings();
		ImGui::Separator();
		DrawJsonOperations();
	}
}

void CameraControllerEditor::DrawCinematicSettings()
{
	ImGui::Text("演出設定:");

	ImGui::Combo("Type", &state_.selectedType, kTypeNames, IM_ARRAYSIZE(kTypeNames));
	ImGui::DragFloat("Duration", &state_.duration, 0.1f, 0.1f, 30.0f);

	ImGui::DragFloat3("Start Position", state_.startPos, 0.5f);
	ImGui::DragFloat3("End Position", state_.endPos, 0.5f);
	ImGui::DragFloat3("Target Position", state_.targetPos, 0.5f);
	ImGui::DragFloat3("Start Rotation", state_.startRot, 0.01f);
	ImGui::DragFloat3("End Rotation", state_.endRot, 0.01f);

	ImGui::DragFloat("Orbit Radius", &state_.orbitRadius, 0.5f, 1.0f, 100.0f);
	ImGui::DragFloat("Orbit Speed", &state_.orbitSpeed, 0.1f, 0.1f, 10.0f);

	ImGui::Checkbox("Use Easing", &state_.useEasing);
	ImGui::Combo("Easing Type", &state_.selectedEasing, kEasingNames, IM_ARRAYSIZE(kEasingNames));

	ImGui::Separator();

	if (ImGui::Button("演出開始", ImVec2(120, 0))) {
		CameraController::CinematicConfig config;
		config.type = static_cast<CameraController::CinematicType>(state_.selectedType);
		config.duration = state_.duration;
		config.startPosition = {state_.startPos[0], state_.startPos[1], state_.startPos[2]};
		config.endPosition = {state_.endPos[0], state_.endPos[1], state_.endPos[2]};
		config.targetPosition = {state_.targetPos[0], state_.targetPos[1], state_.targetPos[2]};
		config.startRotation = {state_.startRot[0], state_.startRot[1], state_.startRot[2]};
		config.endRotation = {state_.endRot[0], state_.endRot[1], state_.endRot[2]};
		config.orbitRadius = state_.orbitRadius;
		config.orbitSpeed = state_.orbitSpeed;
		config.useEasing = state_.useEasing;
		config.easingType = kEasingNames[state_.selectedEasing];
		controller_->StartCinematic(config);
	}

	ImGui::SameLine();

	if (ImGui::Button("現在位置取得", ImVec2(120, 0))) {
		auto pos = controller_->GetCurrentCameraPos();
		auto rot = controller_->GetCurrentCameraRotation();
		state_.startPos[0] = pos.x;
		state_.startPos[1] = pos.y;
		state_.startPos[2] = pos.z;
		state_.startRot[0] = rot.x;
		state_.startRot[1] = rot.y;
		state_.startRot[2] = rot.z;
	}
}

void CameraControllerEditor::DrawJsonOperations()
{
	ImGui::InputText("JSON Path", state_.jsonPath, sizeof(state_.jsonPath));

	if (ImGui::Button("JSONから読込", ImVec2(120, 0))) {
		controller_->StartCinematicFromJson(state_.jsonPath);
	}

	ImGui::SameLine();

	if (ImGui::Button("JSONに保存", ImVec2(120, 0))) {
		controller_->SaveCinematicToJson(state_.jsonPath);
	}
}

void CameraControllerEditor::DrawPresetSequenceSection()
{
	if (ImGui::CollapsingHeader("Preset & Sequence Control")) {
		auto& presetManager = CinematicPresetManager::GetInstance();

		// プリセット選択
		ImGui::Text("=== プリセット演出 ===");
		auto presetNames = presetManager.GetPresetNames();

		if (!presetNames.empty()) {
			const char* currentPreset = state_.selectedPreset < static_cast<int>(presetNames.size()) 
				? presetNames[state_.selectedPreset].c_str() 
				: "なし";

			if (ImGui::BeginCombo("プリセット", currentPreset)) {
				for (int i = 0; i < static_cast<int>(presetNames.size()); i++) {
					bool isSelected = (state_.selectedPreset == i);
					if (ImGui::Selectable(presetNames[i].c_str(), isSelected)) {
						state_.selectedPreset = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("プリセット開始", ImVec2(150, 0))) {
				if (state_.selectedPreset < static_cast<int>(presetNames.size())) {
					controller_->StartCinematicByName(presetNames[state_.selectedPreset]);
				}
			}
		} else {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "プリセットが登録されていません");
		}

		ImGui::Separator();

		// シーケンス選択
		ImGui::Text("=== カット割りシーケンス ===");
		auto sequenceNames = presetManager.GetSequenceNames();

		if (!sequenceNames.empty()) {
			const char* currentSequence = state_.selectedSequence < static_cast<int>(sequenceNames.size())
				? sequenceNames[state_.selectedSequence].c_str()
				: "なし";

			if (ImGui::BeginCombo("シーケンス", currentSequence)) {
				for (int i = 0; i < static_cast<int>(sequenceNames.size()); i++) {
					bool isSelected = (state_.selectedSequence == i);
					if (ImGui::Selectable(sequenceNames[i].c_str(), isSelected)) {
						state_.selectedSequence = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("シーケンス開始", ImVec2(150, 0))) {
				if (state_.selectedSequence < static_cast<int>(sequenceNames.size())) {
					controller_->StartSequenceByName(sequenceNames[state_.selectedSequence]);
				}
			}

			ImGui::SameLine();

			if (controller_->IsSequenceActive()) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "実行中");
				if (ImGui::Button("シーケンス停止", ImVec2(150, 0))) {
					controller_->StopSequence();
				}
			}
		} else {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "シーケンスが登録されていません");
		}
	}
}

void CameraControllerEditor::DrawCameraParametersSection()
{
	ImGui::Text("=== Camera Parameters ===");

	// 距離設定
	float minDistance = controller_->GetMinDistance();
	float maxDistance = controller_->GetMaxDistance();
	float distanceScale = controller_->GetDistanceScale();
	float marginDistance = controller_->GetMarginDistance();

	if (ImGui::DragFloat("Min Distance", &minDistance, 0.1f, 1.0f, 50.0f)) {
		controller_->SetMinDistance(minDistance);
	}
	if (ImGui::DragFloat("Max Distance", &maxDistance, 0.1f, 1.0f, 100.0f)) {
		controller_->SetMaxDistance(maxDistance);
	}
	if (ImGui::DragFloat("Distance Scale", &distanceScale, 0.01f, 0.5f, 5.0f)) {
		controller_->SetDistanceScale(distanceScale);
	}
	if (ImGui::DragFloat("Margin Distance", &marginDistance, 0.1f, 0.0f, 20.0f)) {
		controller_->SetMarginDistance(marginDistance);
	}

	ImGui::Separator();

	// カメラ位置・角度
	float heightOffset = controller_->GetHeightOffset();
	float pitchAngle = controller_->GetPitchAngle();

	if (ImGui::DragFloat("Height Offset", &heightOffset, 0.1f, -10.0f, 20.0f)) {
		controller_->SetHeightOffset(heightOffset);
	}
	if (ImGui::SliderAngle("Pitch Angle", &pitchAngle, 0.0f, 90.0f)) {
		controller_->SetPitchAngle(pitchAngle);
	}

	ImGui::Separator();

	// 画面パディング
	float screenPadding = controller_->GetScreenPadding();
	if (ImGui::SliderFloat("Screen Padding", &screenPadding, 0.0f, 0.4f, "%.2f")) {
		controller_->SetScreenPadding(screenPadding);
	}
	ImGui::TextWrapped("画面端からの余白（0.15 = 15%%）");

	ImGui::Separator();

	// スムーズ設定
	float smoothSpeed = controller_->GetSmoothSpeed();
	if (ImGui::DragFloat("Smooth Speed", &smoothSpeed, 0.1f, 0.1f, 20.0f)) {
		controller_->SetSmoothSpeed(smoothSpeed);
	}
	ImGui::TextWrapped("推奨値: 3.0-8.0 (低いほど滑らか、高いほど反応が速い)");
}

void CameraControllerEditor::DrawStageBoundsSection()
{
	if (ImGui::CollapsingHeader("Stage Bounds")) {
		bool useBounds = controller_->UseStageBounds();
		if (ImGui::Checkbox("Use Stage Bounds", &useBounds)) {
			// ここでは直接設定できないので、現在の境界を取得して再設定
			// controller_->SetUseStageBounds(useBounds);
		}
		ImGui::TextWrapped("ステージ境界制限を有効にすると、カメラがステージ外を映さなくなります");

		if (useBounds) {
			ImGui::Separator();
			ImGui::Text("境界設定:");

			auto bounds = controller_->GetStageBounds();
			float minX = bounds.minX;
			float maxX = bounds.maxX;
			float minY = bounds.minY;
			float maxY = bounds.maxY;

			if (ImGui::DragFloat("Min X", &minX, 0.5f, -200.0f, maxX)) {
				controller_->SetStageBounds(minX, maxX, minY, maxY);
			}
			if (ImGui::DragFloat("Max X", &maxX, 0.5f, minX, 200.0f)) {
				controller_->SetStageBounds(minX, maxX, minY, maxY);
			}
			if (ImGui::DragFloat("Min Y", &minY, 0.5f, -200.0f, maxY)) {
				controller_->SetStageBounds(minX, maxX, minY, maxY);
			}
			if (ImGui::DragFloat("Max Y", &maxY, 0.5f, minY, 200.0f)) {
				controller_->SetStageBounds(minX, maxX, minY, maxY);
			}

			ImGui::Separator();
			ImGui::Text("ステージサイズ:");
			float stageWidth = maxX - minX;
			float stageHeight = maxY - minY;
			ImGui::Text("幅: %.2f", stageWidth);
			ImGui::Text("高さ: %.2f", stageHeight);
		}
	}
}

void CameraControllerEditor::DrawCameraShakeSection()
{
	if (ImGui::CollapsingHeader("Camera Shake")) {
		ImGui::Text("状態: %s", controller_->IsShaking() ? "シェイク中" : "停止中");

		if (controller_->IsShaking()) {
			// 進行度表示は内部実装に依存するため省略
		}

		ImGui::Separator();
		ImGui::Text("プリセット版:");
		ImGui::TextWrapped("ボタンをクリックするだけでシェイクが開始されます");

		if (ImGui::Button("小シェイク (0.3秒)", ImVec2(150, 0))) {
			controller_->StartShake(CameraController::ShakeIntensity::Small);
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "軽い衝撃用");

		if (ImGui::Button("中シェイク (0.5秒)", ImVec2(150, 0))) {
			controller_->StartShake(CameraController::ShakeIntensity::Medium);
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "通常攻撃用");

		if (ImGui::Button("大シェイク (0.8秒)", ImVec2(150, 0))) {
			controller_->StartShake(CameraController::ShakeIntensity::Large);
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "強力攻撃用");

		ImGui::Separator();
		ImGui::Text("カスタム版:");

		ImGui::DragFloat("継続時間", &state_.customDuration, 0.1f, 0.1f, 5.0f);
		ImGui::DragFloat("揺れの大きさ", &state_.customMagnitude, 0.01f, 0.0f, 2.0f);
		ImGui::DragFloat("周波数", &state_.customFrequency, 1.0f, 1.0f, 60.0f);
		ImGui::SliderFloat("減衰率", &state_.customDamping, 0.0f, 1.0f);

		if (ImGui::Button("カスタムシェイク開始", ImVec2(200, 0))) {
			controller_->StartShake(state_.customDuration, state_.customMagnitude, 
				state_.customFrequency, state_.customDamping);
		}

		ImGui::Separator();

		if (ImGui::Button("シェイク停止", ImVec2(100, 0))) {
			controller_->StopShake();
		}
	}
}

void CameraControllerEditor::DrawCurrentStatusSection()
{
	ImGui::Text("=== Current Status ===");

	Vector3 targetPos = controller_->GetTargetPosition();
	ImGui::Text("Target Position: (%.2f, %.2f, %.2f)", targetPos.x, targetPos.y, targetPos.z);

	Vector3 cameraPos = controller_->GetCurrentCameraPos();
	ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);

	if (controller_->IsShaking()) {
		Vector3 shakeOffset = controller_->GetShakeOffset();
		ImGui::Text("Shake Offset: (%.3f, %.3f, %.3f)", shakeOffset.x, shakeOffset.y, shakeOffset.z);
	}

	float currentDistance = controller_->GetCurrentDistance();
	ImGui::Text("Current Distance: %.2f", currentDistance);

	// オブジェクト間距離の表示（GetTargets()が必要なため省略）
	ImGui::Separator();
	ImGui::Text("Aspect Ratio: 16:9 (%.2f)", 16.0f / 9.0f);
	ImGui::Text("FOV Y: %.2f rad (%.1f deg)", 0.45f, 0.45f * 180.0f / 3.14159f);
}

void CameraControllerEditor::DrawSequenceEditorSection()
{
	if (ImGui::CollapsingHeader("Sequence Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("=== シーケンスエディター ===");

		// シーケンス名
		ImGui::InputText("シーケンス名", state_.sequenceName, sizeof(state_.sequenceName));

		ImGui::Separator();
		ImGui::Text("カットリスト:");

		// カットリストの表示
		for (int i = 0; i < static_cast<int>(state_.editingCuts.size()); i++) {
			ImGui::PushID(i);

			auto& cut = state_.editingCuts[i];
			bool nodeOpen = ImGui::TreeNode(cut.name.c_str());

			// カット削除ボタン
			ImGui::SameLine();
			if (ImGui::SmallButton("削除")) {
				state_.editingCuts.erase(state_.editingCuts.begin() + i);
				ImGui::PopID();
				break;
			}

			// カット上下移動
			ImGui::SameLine();
			if (i > 0 && ImGui::SmallButton("↑")) {
				std::swap(state_.editingCuts[i], state_.editingCuts[i - 1]);
			}
			ImGui::SameLine();
			if (i < static_cast<int>(state_.editingCuts.size()) - 1 && ImGui::SmallButton("↓")) {
				std::swap(state_.editingCuts[i], state_.editingCuts[i + 1]);
			}

			if (nodeOpen) {
				// カット名
				char cutName[256];
				strncpy_s(cutName, cut.name.c_str(), sizeof(cutName) - 1);
				if (ImGui::InputText("名前", cutName, sizeof(cutName))) {
					cut.name = cutName;
				}

				// 継続時間
				ImGui::DragFloat("継続時間", &cut.duration, 0.1f, 0.1f, 30.0f);

				// タイプ
				int typeIndex = static_cast<int>(cut.config.type);
				if (ImGui::Combo("Type", &typeIndex, kTypeNames, IM_ARRAYSIZE(kTypeNames))) {
					cut.config.type = static_cast<CameraController::CinematicType>(typeIndex);
				}

				// 位置と回転
				float startPos[3] = {cut.config.startPosition.x, cut.config.startPosition.y, cut.config.startPosition.z};
				if (ImGui::DragFloat3("Start Position", startPos, 0.5f)) {
					cut.config.startPosition = {startPos[0], startPos[1], startPos[2]};
				}

				float endPos[3] = {cut.config.endPosition.x, cut.config.endPosition.y, cut.config.endPosition.z};
				if (ImGui::DragFloat3("End Position", endPos, 0.5f)) {
					cut.config.endPosition = {endPos[0], endPos[1], endPos[2]};
				}

				float targetPos[3] = {cut.config.targetPosition.x, cut.config.targetPosition.y, cut.config.targetPosition.z};
				if (ImGui::DragFloat3("Target Position", targetPos, 0.5f)) {
					cut.config.targetPosition = {targetPos[0], targetPos[1], targetPos[2]};
				}

				float startRot[3] = {cut.config.startRotation.x, cut.config.startRotation.y, cut.config.startRotation.z};
				if (ImGui::DragFloat3("Start Rotation", startRot, 0.01f)) {
					cut.config.startRotation = {startRot[0], startRot[1], startRot[2]};
				}

				float endRot[3] = {cut.config.endRotation.x, cut.config.endRotation.y, cut.config.endRotation.z};
				if (ImGui::DragFloat3("End Rotation", endRot, 0.01f)) {
					cut.config.endRotation = {endRot[0], endRot[1], endRot[2]};
				}

				// Orbit設定
				ImGui::DragFloat("Orbit Radius", &cut.config.orbitRadius, 0.5f, 1.0f, 100.0f);
				ImGui::DragFloat("Orbit Speed", &cut.config.orbitSpeed, 0.1f, 0.1f, 10.0f);

				// イージング設定
				ImGui::Checkbox("Use Easing", &cut.config.useEasing);

				int easingIndex = 3; // デフォルトはEaseInOutQuad
				for (int j = 0; j < IM_ARRAYSIZE(kEasingNames); j++) {
					if (cut.config.easingType == kEasingNames[j]) {
						easingIndex = j;
						break;
					}
				}
				if (ImGui::Combo("Easing Type", &easingIndex, kEasingNames, IM_ARRAYSIZE(kEasingNames))) {
					cut.config.easingType = kEasingNames[easingIndex];
				}

				// このカットをプレビュー
				if (ImGui::Button("このカットをプレビュー", ImVec2(-1, 0))) {
					controller_->StartCinematic(cut.config);
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::Separator();

		// 新規カット追加
		if (ImGui::Button("新規カット追加", ImVec2(150, 0))) {
			CinematicCut newCut;
			newCut.name = "Cut " + std::to_string(state_.editingCuts.size() + 1);
			newCut.duration = 3.0f;
			newCut.config.type = CameraController::CinematicType::FixedPosition;
			newCut.config.duration = 3.0f;
			newCut.config.startPosition = controller_->GetCurrentCameraPos();
			newCut.config.startRotation = controller_->GetCurrentCameraRotation();
			newCut.config.useEasing = true;
			newCut.config.easingType = "EaseInOutQuad";
			state_.editingCuts.push_back(newCut);
		}

		ImGui::SameLine();

		// 全カットクリア
		if (ImGui::Button("全クリア", ImVec2(100, 0))) {
			state_.editingCuts.clear();
		}

		ImGui::Separator();

		// シーケンスの保存と読み込み
		ImGui::InputText("Sequence JSON Path", state_.sequenceJsonPath, sizeof(state_.sequenceJsonPath));

		if (ImGui::Button("JSONから読み込み", ImVec2(150, 0))) {
			auto sequence = std::make_shared<CinematicSequence>();
			if (sequence->LoadFromJson(state_.sequenceJsonPath)) {
				// シーケンスからカットリストを取得
				state_.editingCuts.clear();
				const auto& cuts = sequence->GetCuts();
				for (const auto& cut : cuts) {
					state_.editingCuts.push_back(cut);
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("JSONに保存", ImVec2(150, 0))) {
			auto sequence = std::make_shared<CinematicSequence>();
			for (const auto& cut : state_.editingCuts) {
				sequence->AddCut(cut);
			}
			if (sequence->SaveToJson(state_.sequenceJsonPath)) {
				ImGui::Text("保存成功");
			} else {
				ImGui::Text("保存失敗");
			}
		}

		ImGui::Separator();

		// シーケンスのプレビュー/実行
		if (ImGui::Button("シーケンスをプレビュー", ImVec2(200, 0))) {
			auto sequence = std::make_shared<CinematicSequence>();
			for (const auto& cut : state_.editingCuts) {
				sequence->AddCut(cut);
			}
			controller_->StartSequence(sequence);
		}

		// シーケンスをプリセットマネージャーに登録
		if (ImGui::Button("プリセットに登録", ImVec2(200, 0))) {
			if (strlen(state_.sequenceName) > 0) {
				auto sequence = std::make_shared<CinematicSequence>();
				for (const auto& cut : state_.editingCuts) {
					sequence->AddCut(cut);
				}
				auto& presetManager = CinematicPresetManager::GetInstance();
				presetManager.RegisterSequence(state_.sequenceName, sequence);
				ImGui::Text("登録成功: %s", state_.sequenceName);
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "シーケンス名を入力してください");
			}
		}

		// 合計時間の表示
		float totalDuration = 0.0f;
		for (const auto& cut : state_.editingCuts) {
			totalDuration += cut.duration;
		}
		ImGui::Text("合計時間: %.2f秒 (%d カット)", totalDuration, static_cast<int>(state_.editingCuts.size()));
	}
}

#endif // _DEBUG
