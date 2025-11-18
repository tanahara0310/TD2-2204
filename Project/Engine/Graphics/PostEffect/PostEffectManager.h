#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <d3d12.h>

#include "Engine/Graphics/PostEffect/PostEffectBase.h"
#include "PostEffectPresetManager.h"

class DirectXCommon;
class Render;
class GrayScale;
class FullScreen;
class Blur;
class Shockwave;
class Vignette;
class RadialBlur;
class ColorGrading;
class ChromaticAberration;
class Sepia;
class Invert;
class RasterScroll;
class FadeEffect;

/// @brief ポストエフェクト管理クラス
class PostEffectManager {
public:
    /// @brief 初期化
    /// @param dxCommon DirectXCommonのポインタ
    /// @param render Renderクラスのポインタ
    void Initialize(DirectXCommon* dxCommon, Render* render);

    /// @brief ポストエフェクトを登録
    /// @param name エフェクト名
    /// @param effect ポストエフェクトのインスタンス
    void RegisterEffect(const std::string& name, std::unique_ptr<PostEffectBase> effect);

    /// @brief ポストエフェクトを取得
    /// @param name エフェクト名
    /// @return ポストエフェクトのポインタ（見つからない場合はnullptr）
    PostEffectBase* GetEffect(const std::string& name);

    /// @brief ポストエフェクトを取得（const版）
    /// @param name エフェクト名
    /// @return ポストエフェクトのポインタ（見つからない場合はnullptr）
    const PostEffectBase* GetEffect(const std::string& name) const;

    /// @brief 特定のポストエフェクトを実行（有効/無効チェックなし）
    /// @param name エフェクト名
    /// @param inputSrvHandle 入力テクスチャのSRVハンドル
    void ExecuteEffect(const std::string& name, D3D12_GPU_DESCRIPTOR_HANDLE inputSrvHandle);

    /// @brief エフェクトの有効/無効を設定
    /// @param effectName エフェクト名
    /// @param enabled 有効にするかどうか
    void SetEffectEnabled(const std::string& effectName, bool enabled);

    /// @brief エフェクトが有効かどうかを取得
    /// @param effectName エフェクト名
    /// @return 有効ならtrue
    bool IsEffectEnabled(const std::string& effectName) const;

    /// @brief 更新処理
    /// @param deltaTime フレーム時間
    void Update(float deltaTime);

    /// @brief ImGuiでポストエフェクトのパラメータを調整
    void DrawImGui();

    /// @brief プリセットマネージャーを取得
    /// @return プリセットマネージャーの参照
    PostEffectPresetManager& GetPresetManager() { return *presetManager_; }

    /// @brief GrayScaleエフェクトを取得
    GrayScale* GetGrayScale();

    /// @brief FullScreenエフェクトを取得
    FullScreen* GetFullScreen();

    /// @brief Blurエフェクトを取得
    Blur* GetBlur();

    /// @brief Shockwaveエフェクトを取得
    Shockwave* GetShockwave();

    /// @brief Vignetteエフェクトを取得
    Vignette* GetVignette();

    /// @brief RadialBlurエフェクトを取得
    RadialBlur* GetRadialBlur();

    /// @brief ColorGradingエフェクトを取得
    ColorGrading* GetColorGrading();

    /// @brief ChromaticAberrationエフェクトを取得
    ChromaticAberration* GetChromaticAberration();

    /// @brief Sepiaエフェクトを取得
    Sepia* GetSepia();

    /// @brief Invertエフェクトを取得
    Invert* GetInvert();

    /// @brief RasterScrollエフェクトを取得
    RasterScroll* GetRasterScroll();

    /// @brief FadeEffectエフェクトを取得
    FadeEffect* GetFadeEffect();

    /// @brief 現在表示すべき最終テクスチャハンドルを取得
    /// @return 表示すべきテクスチャのSRVハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE GetFinalDisplayTextureHandle() const;

    /// @brief デフォルトエフェクトチェーンを実行し、結果のテクスチャハンドルを取得
    /// @param inputSrvHandle 入力テクスチャのSRVハンドル
    /// @return 最終出力のSRVハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE ExecuteDefaultEffectChain(D3D12_GPU_DESCRIPTOR_HANDLE inputSrvHandle);

private:
    /// @brief Ping-Pongバッファ管理用ヘルパークラス
    class PingPongBuffer {
    public:
      PingPongBuffer(DirectXCommon* dxCommon, Render* render);

        /// @brief エフェクトを適用し、バッファを切り替える
        /// @param effect 適用するエフェクト
        /// @return 適用されたかどうか
        bool ApplyEffect(PostEffectBase* effect);

        /// @brief 現在の出力SRVハンドルを取得
        D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentOutput() const;

        /// @brief 最終結果をオフスクリーン#1に保証する
        /// @param fullScreenEffect FullScreenエフェクト（コピー用）
        void EnsureOutputInBuffer1(FullScreen* fullScreenEffect);

        /// @brief 入力をリセット
        void Reset(D3D12_GPU_DESCRIPTOR_HANDLE input);

    private:
        DirectXCommon* dxCommon_;
        Render* render_;
      D3D12_GPU_DESCRIPTOR_HANDLE currentInput_;
        int currentOutputIndex_;

      D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle(int index) const;
    };

    /// @brief 有効なエフェクトの名前リストを収集
    /// @param effectNames エフェクト名のリスト
    /// @return 有効なエフェクト名のリスト
    std::vector<std::string> CollectEnabledEffectNames(const std::vector<std::string>& effectNames) const;

    DirectXCommon* directXCommon_ = nullptr;
    Render* render_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<PostEffectBase>> effects_;
    
    // エフェクトチェーンの定義
    std::vector<std::string> defaultEffectChain_ = { 
        "FadeEffect", "Shockwave", "Blur", "RadialBlur", "RasterScroll", 
        "ColorGrading", "ChromaticAberration", "Sepia", "Invert", "GrayScale", "Vignette" 
    };

    // 個別エフェクトへの参照（高速アクセス用）
    GrayScale* grayScale_ = nullptr;
    FullScreen* fullScreen_ = nullptr;
    Blur* blur_ = nullptr;
Shockwave* shockwave_ = nullptr;
    Vignette* vignette_ = nullptr;
    RadialBlur* radialBlur_ = nullptr;
    ColorGrading* colorGrading_ = nullptr;
 ChromaticAberration* chromaticAberration_ = nullptr;
    Sepia* sepia_ = nullptr;
    Invert* invert_ = nullptr;
    RasterScroll* rasterScroll_ = nullptr;
    FadeEffect* fadeEffect_ = nullptr;
    
    // プリセット管理
    std::unique_ptr<PostEffectPresetManager> presetManager_;
    
    // 最終表示テクスチャハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE finalDisplayHandle_;
};