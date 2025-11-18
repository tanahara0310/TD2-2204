#include "ResourceFactory.h"
#include <cassert>
#include <stdexcept>

Microsoft::WRL::ComPtr<ID3D12Resource> ResourceFactory::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
    // 頂点リソース用のヒープの設定
    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    // 頂点リソースの設定
    D3D12_RESOURCE_DESC resourceDesc {};
    // バッファリソース。テクスチャの場合は別の設定をする
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    size_t alignedSize = (sizeInBytes + 255) & ~0xFF;
    resourceDesc.Width = alignedSize;
    // バッファの場合は1
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    // バッファの場合はこれにする仕組み
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    // 頂点リソースを生成
    Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&bufferResource));

    // リソースの生成が上手く行かなかったので起動できない
    if (FAILED(hr)) {
		throw std::runtime_error("Failed to create BufferResource");

    }


    return bufferResource;
}
