#pragma once

#include "MathCore.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>

// 定数バッファ用データ
struct ConstantBufferDataWorldTransform {
    Matrix4x4 matWorld; // ワールド変換行列
};

/// <summary>
/// ワールドトランスフォームクラス
/// 3Dオブジェクトの位置・回転・スケールを管理し、GPU用の行列を生成する
/// </summary>
class WorldTransform {
public:
    // === トランスフォームパラメータ（直接アクセス可能） ===
    Vector3 scale = { 1.0f, 1.0f, 1.0f };      // スケール
    Vector3 rotate = { 0.0f, 0.0f, 0.0f };     // 回転角（ラジアン）
    Vector3 translate = { 0.0f, 0.0f, 0.0f };  // 位置

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void Initialize(ID3D12Device* device);

    /// <summary>
    /// ワールド行列を計算してGPUに転送
    /// 毎フレーム描画前に呼び出す
    /// </summary>
    void TransferMatrix();

    /// <summary>
    /// ImGuiでTransform情報を表示・編集（デバッグ用）
    /// </summary>
    /// <param name="label">ラベル名</param>
    /// <returns>変更があった場合true</returns>
    bool DrawImGui(const std::string& label);

    /// <summary>
    /// GPU仮想アドレスを取得
    /// </summary>
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    /// <summary>
    /// 計算済みワールド行列を取得
    /// </summary>
    const Matrix4x4& GetWorldMatrix() const { return matWorld_; }

    /// <summary>
    /// ワールド座標での位置を取得
    /// </summary>
    Vector3 GetWorldPosition() const;

    /// <summary>
    /// 親トランスフォームを設定（階層構造用）
    /// </summary>
    /// <param name="parent">親トランスフォームのポインタ（nullptrで親なし）</param>
    void SetParent(const WorldTransform* parent) { parent_ = parent; }

    /// <summary>
    /// 親トランスフォームを取得
    /// </summary>
    const WorldTransform* GetParent() const { return parent_; }

    /// <summary>
    /// ワールド行列を直接設定（アニメーション用）
    /// </summary>
    /// <param name="matrix">設定する行列</param>
    void SetWorldMatrix(const Matrix4x4& matrix);

private:
    // 定数バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    // マッピング済みポインタ
    ConstantBufferDataWorldTransform* mapped_ = nullptr;
    // 計算済みワールド行列
    Matrix4x4 matWorld_;
    // 親トランスフォーム（階層構造用）
    const WorldTransform* parent_ = nullptr;
};
