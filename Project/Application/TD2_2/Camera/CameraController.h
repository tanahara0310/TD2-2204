#pragma once

#include "Camera/Release/Camera.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include <memory>

/// @brief 大乱闘スマッシュブラザーズスタイルのカメラコントローラー
/// @details 2つのゲームオブジェクトの中点を注視し、距離に応じてカメラを自動調整
class CameraController {
public:
	/// @brief カメラシェイクの強度プリセット
	enum class ShakeIntensity {
		Small,   ///< 小（軽い揺れ）
		Medium,  ///< 中（標準的な揺れ）
		Large    ///< 大（激しい揺れ）
	};

	/// @brief コンストラクタ
	CameraController() = default;

	/// @brief デストラクタ
	~CameraController() = default;

	/// @brief 初期化
	/// @param camera 制御するカメラ
	/// @param object1 追跡するゲームオブジェクト1
	/// @param object2 追跡するゲームオブジェクト2
	void Initialize(Camera* camera, GameObject* object1, GameObject* object2);

	/// @brief 更新処理
	void Update();

	/// @brief カメラシェイクを開始（カスタムパラメータ版）
	/// @param duration 継続時間（秒）
	/// @param magnitude 揺れの大きさ
	/// @param frequency 揺れの周波数（1秒間の揺れ回数）
	/// @param damping 減衰率（0.0-1.0、大きいほど早く減衰）
	void StartShake(float duration, float magnitude, float frequency = 20.0f, float damping = 0.8f);

	/// @brief カメラシェイクを開始（プリセット版 - 継続時間も事前設定）
	/// @param intensity 揺れの強度（Small/Medium/Large）
	void StartShake(ShakeIntensity intensity);

	/// @brief カメラシェイクを停止
	void StopShake();

	/// @brief カメラシェイクが実行中かどうか
	/// @return 実行中の場合true
	bool IsShaking() const;

	/// @brief カメラ設定のアクセッサ

	/// @brief 最小距離を設定
	/// @param distance カメラとターゲット間の最小距離
	void SetMinDistance(float distance) { minDistance_ = distance; }

	/// @brief 最大距離を設定
	/// @param distance カメラとターゲット間の最大距離
	void SetMaxDistance(float distance) { maxDistance_ = distance; }

	/// @brief 距離スケールを設定
	/// @param scale オブジェクト間距離とカメラ距離の比率
	void SetDistanceScale(float scale) { distanceScale_ = scale; }

	/// @brief カメラの高さオフセットを設定
	/// @param offset Y軸方向のオフセット
	void SetHeightOffset(float offset) { heightOffset_ = offset; }

	/// @brief カメラの俯角を設定
	/// @param angle 俯角（ラジアン）
	void SetPitchAngle(float angle) { pitchAngle_ = angle; }

	/// @brief スムーズ補間速度を設定
	/// @param speed 補間速度（大きいほど速く追従）
	void SetSmoothSpeed(float speed) { smoothSpeed_ = speed; }

	/// @brief マージン距離を設定（この距離内は無視）
	/// @param margin マージン距離
	void SetMarginDistance(float margin) { marginDistance_ = margin; }

	/// @brief 画面パディングを設定
	/// @param padding 画面端からの余白（0.0-1.0、デフォルト0.1 = 10%）
	void SetScreenPadding(float padding) { screenPadding_ = padding; }

	/// @brief 追跡対象を設定
	/// @param object1 追跡するゲームオブジェクト1
	/// @param object2 追跡するゲームオブジェクト2
	void SetTargets(GameObject* object1, GameObject* object2);

	/// @brief 現在の注視点を取得
	/// @return 注視点の座標
	Vector3 GetTargetPosition() const { return targetPosition_; }

	/// @brief 現在のカメラ距離を取得
	/// @return カメラの距離
	float GetCurrentDistance() const { return currentDistance_; }

#ifdef _DEBUG
	/// @brief ImGuiデバッグUI
	void DrawImGui();
#endif

private:
	/// @brief ターゲット位置を計算（2つのオブジェクトの中点）
	/// @return 中点座標
	Vector3 CalculateTargetPosition() const;

	/// @brief オブジェクト間の距離を計算
	/// @return 2つのオブジェクト間の距離
	float CalculateObjectDistance() const;

	/// @brief オブジェクト間の横幅を計算（X軸方向）
	/// @return 横幅
	float CalculateHorizontalDistance() const;

	/// @brief オブジェクト間の縦幅を計算（Y軸方向）
	/// @return 縦幅
	float CalculateVerticalDistance() const;

	/// @brief 必要なカメラ距離を計算（前後・横幅・縦幅すべてを考慮）
	/// @param objectDistance オブジェクト間の3D距離
	/// @param horizontalDistance 横方向の距離
	/// @param verticalDistance 縦方向の距離
	/// @return 必要なカメラ距離
	float CalculateRequiredDistance(float objectDistance, float horizontalDistance, float verticalDistance) const;

	/// @brief カメラの距離を計算（オブジェクト間距離とアスペクト比に基づく）
	/// @param objectDistance オブジェクト間の距離
	/// @return カメラの距離
	float CalculateCameraDistance(float objectDistance) const;

	/// @brief カメラの位置を計算
	/// @param targetPos 注視点
	/// @param distance カメラの距離
	/// @return カメラの位置
	Vector3 CalculateCameraPosition(const Vector3& targetPos, float distance) const;

	/// @brief カメラの回転を計算
	/// @return カメラの回転（オイラー角）
	Vector3 CalculateCameraRotation() const;

	/// @brief カメラシェイクの更新
	/// @param deltaTime デルタタイム
	void UpdateShake(float deltaTime);

	/// @brief シェイクオフセットを計算
	/// @return シェイクによるオフセット
	Vector3 CalculateShakeOffset() const;

	// 制御対象
	Camera* camera_ = nullptr;              ///< 制御するカメラ
	GameObject* object1_ = nullptr;         ///< 追跡対象1
	GameObject* object2_ = nullptr;         ///< 追跡対象2

	// カメラパラメータ
	float minDistance_ = 10.0f;             ///< 最小カメラ距離
	float maxDistance_ = 30.0f;             ///< 最大カメラ距離
	float distanceScale_ = 1.5f;            ///< オブジェクト間距離に対するカメラ距離の倍率
	float heightOffset_ = 3.0f;             ///< カメラの高さオフセット
	float pitchAngle_ = 0.4f;               ///< カメラの俯角（ラジアン）
	float smoothSpeed_ = 4.0f;              ///< スムーズ補間速度（カメラ酔い防止のため低めに設定）
	float marginDistance_ = 5.0f;           ///< この距離内は無視するマージン
	float screenPadding_ = 0.15f;           ///< 画面端からの余白（0.15 = 15%）

	// アスペクト比設定
	static constexpr float kAspectRatio = 16.0f / 9.0f;  ///< アスペクト比（16:9）
	static constexpr float kFovY = 0.45f;                ///< 垂直視野角（ラジアン）

	// 現在の状態
	Vector3 targetPosition_ = { 0.0f, 0.0f, 0.0f };   ///< 現在の注視点
	Vector3 currentCameraPos_ = { 0.0f, 0.0f, 0.0f }; ///< 現在のカメラ位置
	float currentDistance_ = 15.0f;                    ///< 現在のカメラ距離

	// カメラシェイク関連
	GameTimer shakeTimer_;                  ///< シェイク用タイマー
	float shakeMagnitude_ = 0.0f;           ///< 揺れの大きさ
	float shakeFrequency_ = 20.0f;          ///< 揺れの周波数
	float shakeDamping_ = 0.8f;             ///< 減衰率
	float shakeTime_ = 0.0f;                ///< シェイクの経過時間
	Vector3 shakeOffset_ = { 0.0f, 0.0f, 0.0f }; ///< シェイクによるオフセット
};
