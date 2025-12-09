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
    void SetTargetPosition(const Vector3& target) { target_ = target; }

private:
    // カメラパラメータ
    Vector3 cameraPos_ = { 0.0f, -11.0f, -71.8f };  // カメラ位置
    Vector3 target_ = { 0.0f, 20.54f, -17.61f };     // 注視点
    Vector3 up_ = { 0.0f, 1.0f, 0.0f };               // 上方向ベクトル
};
