#pragma once

#include "IRenderer.h"
#include "RenderPassType.h"
#include "Engine/Graphics/PipelineStateManager.h"
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>

// 前方宣言
class IDrawable;
class ICamera;

/// @brief レンダリング全体を自動管理するマネージャー
class RenderManager {
public:
    /// @brief 初期化
    /// @param device D3D12デバイス
    void Initialize(ID3D12Device* device);
    
    /// @brief レンダラーを登録
    /// @param type 描画パスタイプ
    /// @param renderer レンダラーのユニークポインタ
    void RegisterRenderer(RenderPassType type, std::unique_ptr<IRenderer> renderer);
    
    /// @brief レンダラーを取得
    /// @param type 描画パスタイプ
    /// @return レンダラーポインタ
    IRenderer* GetRenderer(RenderPassType type);
    
    /// @brief カメラを設定（フレームごとに1回）
    /// @param camera カメラオブジェクト
    void SetCamera(const ICamera* camera);
    
    /// @brief コマンドリストを設定（フレームごとに1回）
    /// @param cmdList コマンドリスト
    void SetCommandList(ID3D12GraphicsCommandList* cmdList);
    
    /// @brief 描画対象オブジェクトをキューに追加
    /// @param obj 描画するオブジェクト
    void AddDrawable(IDrawable* obj);
    
    /// @brief キューに登録された全オブジェクトを描画
    void DrawAll();
    
    /// @brief フレーム終了時にキューをクリア
    void ClearQueue();
    
private:
    struct DrawCommand {
        IDrawable* object;
        RenderPassType passType;
    };
    
    std::vector<DrawCommand> drawQueue_;
    std::unordered_map<RenderPassType, std::unique_ptr<IRenderer>> renderers_;
    
    // フレームごとに設定されるコンテキスト
    ID3D12GraphicsCommandList* cmdList_ = nullptr;
    const ICamera* camera_ = nullptr;
    
    /// @brief 描画パスごとにソート
    void SortDrawQueue();
};
