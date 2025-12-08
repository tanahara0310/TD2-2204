#pragma once

#include "Camera/Release/Camera.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include <memory>
#include <string>

// 前方宣言
class CinematicSequence;
#ifdef _DEBUG
class CameraControllerEditor;
#endif

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

	/// @brief カメラ演出の種類
	enum class CinematicType {
		None,           ///< 演出なし（通常追従モード）
		FixedPosition,  ///< 固定位置カメラ
		LookAt,         ///< 特定位置を注視
		Dolly,          ///< 移動演出（開始位置→終了位置）
		Arc,            ///< 円弧移動
		Orbit           ///< 対象の周りを回転
	};

	/// @brief カメラ演出の設定
	struct CinematicConfig {
		CinematicType type = CinematicType::None;
		float duration = 3.0f;              ///< 演出の継続時間
		Vector3 startPosition = {0, 0, 0};  ///< 開始位置
		Vector3 endPosition = {0, 0, 0};    ///< 終了位置
		Vector3 targetPosition = {0, 0, 0}; ///< 注視点
		Vector3 startRotation = {0, 0, 0};  ///< 開始回転
		Vector3 endRotation = {0, 0, 0};    ///< 終了回転
		float orbitRadius = 10.0f;          ///< 回転半径
		float orbitSpeed = 1.0f;            ///< 回転速度
		bool useEasing = true;              ///< イージング使用
		std::string easingType = "EaseInOutQuad"; ///< イージングタイプ
	};

	/// @brief コンストラクタ
	CameraController() = default;

	/// @brief デストラクタ
	~CameraController();

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

	// カメラ演出関連

	/// @brief カメラ演出を開始
	/// @param config 演出設定
	void StartCinematic(const CinematicConfig& config);

	/// @brief カメラ演出を停止（通常追従モードに戻る）
	void StopCinematic();

	/// @brief カメラ演出が実行中かどうか
	/// @return 実行中の場合true
	bool IsCinematicActive() const;

	/// @brief カメラ演出の進行度を取得
	/// @return 進行度（0.0～1.0）
	float GetCinematicProgress() const;

	/// @brief JSONファイルからカメラ演出設定を読み込んで開始
	/// @param jsonPath JSONファイルのパス
	/// @return 読み込みに成功した場合true
	bool StartCinematicFromJson(const std::string& jsonPath);

	/// @brief 現在のカメラ演出設定をJSONに保存
	/// @param jsonPath JSONファイルのパス
	/// @return 保存に成功した場合true
	bool SaveCinematicToJson(const std::string& jsonPath) const;

	/// @brief 演出を名前で開始（プリセット使用）
	/// @param presetName プリセット名
	/// @return 成功した場合true
	bool StartCinematicByName(const std::string& presetName);

	/// @brief カット割りシーケンスを開始
	/// @param sequence シーケンス
	void StartSequence(std::shared_ptr<CinematicSequence> sequence);

	/// @brief カット割りシーケンスを名前で開始
	/// @param sequenceName シーケンス名
	/// @return 成功した場合true
	bool StartSequenceByName(const std::string& sequenceName);

	/// @brief シーケンスが実行中かどうか
	/// @return 実行中の場合true
	bool IsSequenceActive() const;

	/// @brief シーケンスを停止
	void StopSequence();

	// カメラ設定のアクセッサー

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

	/// @brief ステージ境界を設定
	/// @param minX ステージの最小X座標
	/// @param maxX ステージの最大X座標
	/// @param minY ステージの最小Y座標
	/// @param maxY ステージの最大Y座標
	void SetStageBounds(float minX, float maxX, float minY, float maxY);

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

	/// @brief 現在のカメラ位置を取得
	/// @return カメラ位置
	Vector3 GetCurrentCameraPos() const { return currentCameraPos_; }

	/// @brief 現在のカメラ回転を取得
	/// @return カメラ回転
	Vector3 GetCurrentCameraRotation() const { return CalculateCameraRotation(); }

	/// @brief シェイクオフセットを取得
	/// @return シェイクオフセット
	Vector3 GetShakeOffset() const { return shakeOffset_; }

	/// @brief 最小距離を取得
	float GetMinDistance() const { return minDistance_; }

	/// @brief 最大距離を取得
	float GetMaxDistance() const { return maxDistance_; }

	/// @brief 距離スケールを取得
	float GetDistanceScale() const { return distanceScale_; }

	/// @brief マージン距離を取得
	float GetMarginDistance() const { return marginDistance_; }

	/// @brief 高さオフセットを取得
	float GetHeightOffset() const { return heightOffset_; }

	/// @brief 俯角を取得
	float GetPitchAngle() const { return pitchAngle_; }

	/// @brief スムーズ速度を取得
	float GetSmoothSpeed() const { return smoothSpeed_; }

	/// @brief 画面パディングを取得
	float GetScreenPadding() const { return screenPadding_; }

	/// @brief ステージ境界使用フラグを取得
	bool UseStageBounds() const { return useStageBounds_; }

	/// @brief ステージ境界を取得
	struct StageBounds {
		float minX, maxX, minY, maxY;
	};
	StageBounds GetStageBounds() const {
		return {stageBoundsMinX_, stageBoundsMaxX_, stageBoundsMinY_, stageBoundsMaxY_};
	}

#ifdef _DEBUG
	/// @brief ImGuiデバッグUI
	void DrawImGui();
#endif

private:
	/// @brief ターゲット位置を計算（2つのオブジェクトの中点）
	/// @return 中点座標
	Vector3 CalculateTargetPosition() const;

	/// @brief プレイヤーを優先したターゲット位置を計算（最大距離到達時用）
	/// @param midpoint 中点座標
	/// @param cameraDistance カメラの距離
	/// @return プレイヤー寄りのターゲット位置
	Vector3 CalculatePlayerPriorityTargetPosition(const Vector3& midpoint, float cameraDistance) const;

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

	/// @brief ステージ境界内にターゲット位置を制限
	/// @param targetPos 制限前のターゲット位置
	/// @param cameraDistance カメラの距離
	/// @return ステージ境界内に制限されたターゲット位置
	Vector3 ClampTargetToStageBounds(const Vector3& targetPos, float cameraDistance) const;

	/// @brief ステージ境界内にカメラ位置を制限（シェイク対応版）
	/// @param cameraPos 制限前のカメラ位置
	/// @param cameraDistance カメラの距離
	/// @return ステージ境界内に制限されたカメラ位置
	Vector3 ClampCameraToStageBounds(const Vector3& cameraPos, float cameraDistance) const;

	/// @brief カメラ演出の更新
	/// @param deltaTime デルタタイム
	void UpdateCinematic(float deltaTime);

	/// @brief 通常追従モードの更新
	void UpdateNormalMode();

	/// @brief 注視点を計算（回転を考慮）
	/// @param position カメラ位置
	/// @param rotation カメラ回転
	/// @return 注視点
	Vector3 CalculateLookAtTarget(const Vector3& position, const Vector3& rotation) const;

	/// @brief イージングタイプ文字列からイージング列挙型に変換
	/// @param typeStr イージングタイプ文字列
	/// @return イージング列挙型
	EasingUtil::Type GetEasingTypeFromString(const std::string& typeStr) const;

	/// @brief シーケンスの更新
	/// @param deltaTime デルタタイム
	void UpdateSequence(float deltaTime);

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

	// ステージ境界
	float stageBoundsMinX_ = -50.0f;        ///< ステージの最小X座標
	float stageBoundsMaxX_ = 50.0f;         ///< ステージの最大X座標
	float stageBoundsMinY_ = -50.0f;        ///< ステージの最小Y座標
	float stageBoundsMaxY_ = 50.0f;         ///< ステージの最大Y座標
	bool useStageBounds_ = false;           ///< ステージ境界制限を使用するか

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

	// カメラ演出関連
	bool cinematicActive_ = false;          ///< 演出が実行中か
	CinematicConfig cinematicConfig_;       ///< 現在の演出設定
	GameTimer cinematicTimer_;              ///< 演出用タイマー
	float orbitAngle_ = 0.0f;               ///< 回転角度（Orbitモード用）

	// シーケンス関連
	std::shared_ptr<CinematicSequence> activeSequence_; ///< 実行中のシーケンス

#ifdef _DEBUG
	friend class CameraControllerEditor;
	CameraControllerEditor* editor_ = nullptr;  ///< ImGuiエディター
#endif
};
