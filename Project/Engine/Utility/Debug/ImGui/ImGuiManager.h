#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <wrl.h>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include "DockingUI.h"
#include "SceneViewport.h"

// 前方宣言
class DirectXCommon;
class PostEffectManager;

/// @brief ImGui管理クラス
class ImGuiManager {
public:
    /// @brief 初期化
    /// @param hwnd
    /// @param device
    /// @param swaoChainDesc
    /// @param srvHeap
    void Initialize(HWND hwnd, DirectXCommon* dxCommon);

    /// @brief ImGuiの開始処理
    /// @param postEffectManager PostEffectManagerへのポインタ（オプション）
    void Begin(PostEffectManager* postEffectManager = nullptr);

    /// @brief ImGuiの終了処理
    void End();

    /// @brief ImGuiの描画
    /// @param commandList
    void Draw();

    /// @brief 終了処理
    void Finalize();

    /// @brief ドッキングUIへのアクセッサ
    /// @return ドッキングUIへのポインタ
    DockingUI* GetDockingUI() const { return dockingUI_.get(); }

    /// @brief シーンビューポートへのアクセッサ
    /// @return シーンビューポートへのポインタ
    SceneViewport* GetSceneViewport() const { return sceneViewport_.get(); }

private:
    HWND hwnd_ = nullptr; // ウィンドウハンドル
    DirectXCommon* dxCommon_ = nullptr; // DirectX共通クラスへのポインタ

    // サブモジュール
    std::unique_ptr<DockingUI> dockingUI_ = std::make_unique<DockingUI>();
    std::unique_ptr<SceneViewport> sceneViewport_ = std::make_unique<SceneViewport>();

private: // メンバ関数
    /// @brief レイアウトや見た目を変更
    void ApplyCustomTheme();

    /// @brief フレームの開始
    void StartNewFrame();
};
