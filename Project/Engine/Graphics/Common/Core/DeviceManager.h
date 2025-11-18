#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class WinApp;

/// @brief DirectX12デバイスとDXGIファクトリの管理クラス
class DeviceManager {
public:
    /// @brief 初期化
    /// @param winApp ウィンドウアプリケーション
    void Initialize(WinApp* winApp);

    // アクセッサ
    ID3D12Device* GetDevice() const { return device_.Get(); }
    IDXGIFactory7* GetDXGIFactory() const { return dxgiFactory_.Get(); }

private:
    /// @brief DXGIデバイスの初期化
    void InitializeDXGIDevice();

private:
    // DXGIファクトリとデバイス
    ComPtr<IDXGIFactory7> dxgiFactory_;
    ComPtr<ID3D12Device> device_;

    WinApp* winApp_ = nullptr;
};