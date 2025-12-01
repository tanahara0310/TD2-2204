#include "ParticleResourceManager.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Particle/ParticleSystem.h" // ParticleForGPU定義のため

void ParticleResourceManager::Initialize(DirectXCommon* dxCommon, ResourceFactory* resourceFactory, uint32_t maxInstances) {
	dxCommon_ = dxCommon;
	resourceFactory_ = resourceFactory;

	// リソースの作成
	CreateInstancingResource(maxInstances);
	CreateSRV(maxInstances);
}

void ParticleResourceManager::CreateInstancingResource(uint32_t maxInstances) {
	// インスタンシング用のリソースを作成
	instancingResource_ = resourceFactory_->CreateBufferResource(
		dxCommon_->GetDevice(),
		sizeof(ParticleForGPU) * maxInstances
	);

	// マップしてポインタを取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
}

void ParticleResourceManager::CreateSRV(uint32_t maxInstances) {
	// インスタンシング用のSRVを設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxInstances;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	// SRVの作成
	dxCommon_->GetDescriptorManager()->CreateSRV(
		instancingResource_.Get(),
		srvDesc,
		srvHandleCPU_,
		srvHandleGPU_,
		"ParticleInstancingSRV"
	);
}
