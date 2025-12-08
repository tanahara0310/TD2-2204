#pragma once

#ifdef _DEBUG

#include "CameraController.h"
#include "CinematicSequence.h"
#include <string>
#include <vector>

/// @brief CameraControllerのImGuiエディター機能
class CameraControllerEditor {
public:
	/// @brief コンストラクタ
	/// @param controller 編集対象のCameraController
	explicit CameraControllerEditor(CameraController* controller);

	/// @brief ImGuiデバッグUI全体の描画
	void DrawImGui();

private:
	/// @brief 演出コントロールセクションの描画
	void DrawCinematicControlSection();

	/// @brief プリセット/シーケンス選択セクションの描画
	void DrawPresetSequenceSection();

	/// @brief カメラパラメータセクションの描画
	void DrawCameraParametersSection();

	/// @brief ステージ境界セクションの描画
	void DrawStageBoundsSection();

	/// @brief カメラシェイクセクションの描画
	void DrawCameraShakeSection();

	/// @brief 現在のステータス表示セクションの描画
	void DrawCurrentStatusSection();

	/// @brief 演出設定UIの描画
	void DrawCinematicSettings();

	/// @brief JSON操作UIの描画
	void DrawJsonOperations();

	/// @brief シーケンスエディターセクションの描画
	void DrawSequenceEditorSection();

private:
	CameraController* controller_ = nullptr;  ///< 編集対象のコントローラー

	// ImGui用の静的変数
	struct EditorState {
		int selectedType = 0;
		float duration = 3.0f;
		float startPos[3] = {0, 0, -50};
		float endPos[3] = {0, 0, -50};
		float targetPos[3] = {0, 0, 0};
		float startRot[3] = {0, 0, 0};
		float endRot[3] = {0, 0, 0};
		float orbitRadius = 10.0f;
		float orbitSpeed = 1.0f;
		bool useEasing = true;
		int selectedEasing = 3;
		char jsonPath[256] = "Resources/Data/CameraCinematic.json";

		// カスタムシェイク設定
		float customDuration = 1.0f;
		float customMagnitude = 0.3f;
		float customFrequency = 20.0f;
		float customDamping = 0.8f;

		// プリセット/シーケンス選択
		int selectedPreset = 0;
		int selectedSequence = 0;

		// シーケンスエディター用
		char sequenceName[256] = "MySequence";
		char sequenceJsonPath[256] = "Resources/Data/CameraSequence.json";
		std::vector<CinematicCut> editingCuts;
	};

	EditorState state_;  ///< エディターの状態

	// 定数配列
	static constexpr const char* kTypeNames[] = {
		"None", "FixedPosition", "LookAt", "Dolly", "Arc", "Orbit"
	};

	static constexpr const char* kEasingNames[] = {
		"Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad",
		"EaseInCubic", "EaseOutCubic", "EaseInOutCubic",
		"EaseInQuart", "EaseOutQuart", "EaseInOutQuart",
		"EaseInQuint", "EaseOutQuint", "EaseInOutQuint",
		"EaseInBack", "EaseOutBack", "EaseInOutBack"
	};
};

#endif // _DEBUG
