#pragma once

#include "MathCore.h"

class Camera;

/// <summary>
/// タイトルシーン専用のカメラコントローラー
/// ImGuiを使ってカメラパラメータをリアルタイムに調整可能
/// </summary>
class TitleCameraController {
public:
    TitleCameraController() = default;
    ~TitleCameraController() = default;

    /// <summary>
    /// カメラパラメータを適用
    /// </summary>
    /// <param name="camera">適用するカメラ</param>
    void ApplyToCamera(Camera* camera);
    
    /// <summary>
    /// 更新処理（ターゲット移動アニメーション）
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    void Update(float deltaTime);

    /// <summary>
    /// ImGuiでパラメータを編集
    /// </summary>
    /// <returns>パラメータが変更された場合true</returns>
    bool DrawImGui();

    /// <summary>
    /// カメラ位置を取得
    /// </summary>
    const Vector3& GetCameraPosition() const { return cameraPos_; }

    /// <summary>
    /// 注視点を取得
    /// </summary>
    const Vector3& GetTargetPosition() const { return target_; }

    /// <summary>
    /// カメラ位置を設定
    /// </summary>
    void SetCameraPosition(const Vector3& pos) { cameraPos_ = pos; }

    /// <summary>
    /// 注視点を設定
    /// </summary>
    void SetTargetPosition(const Vector3& target) { target_ = target; baseTarget_ = target; }
    
    /// <summary>
    /// ターゲット移動アニメーションを開始
    /// </summary>
    void StartTargetAnimation() { isTargetAnimating_ = true; }
    
    /// <summary>
    /// ターゲット移動アニメーションを停止
    /// </summary>
    void StopTargetAnimation() { isTargetAnimating_ = false; }

private:
    // カメラパラメータ
    Vector3 cameraPos_ = { 0.0f, -11.0f, -71.8f };  // カメラ位置
    Vector3 target_ = { 0.0f, 20.54f, -17.61f };     // 注視点
    Vector3 baseTarget_ = { 0.0f, 20.54f, -17.61f }; // 基準となる注視点
    Vector3 up_ = { 0.0f, 1.0f, 0.0f };               // 上方向ベクトル
    
    // ターゲット移動アニメーション用
    bool isTargetAnimating_ = false;
    float targetAnimTimer_ = 0.0f;
    static constexpr float kTargetAnimSpeed = 1.2f;       // アニメーション速度
    static constexpr float kTargetAnimAmplitude = 0.5f;   // 上下移動の振幅
};
