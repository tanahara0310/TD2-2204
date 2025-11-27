#include "Model.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Graphics/Render/Model/ModelRenderer.h"
#include "Engine/Graphics/Render/Model/SkinnedModelRenderer.h"
#include "Engine/Graphics/Model/Skeleton/SkeletonAnimator.h"
#include "Engine/Graphics/Model/Skeleton/SkinClusterGenerator.h"

#include <cassert>

namespace {
	DirectXCommon* sDxCommon_ = nullptr;
	ResourceFactory* sResourceFactory_ = nullptr;
}

void Model::Initialize(DirectXCommon* dxCommon, ResourceFactory* factory) {
	assert(dxCommon && factory);
	sDxCommon_ = dxCommon;
	sResourceFactory_ = factory;
}

void Model::Initialize(ModelResource* resource) {
	assert(resource && resource->IsLoaded());
	resource_ = resource;

	// マテリアルマネージャーを作成
	materialManager_ = std::make_unique<MaterialManager>();
	materialManager_->Initialize(sDxCommon_->GetDevice(), sResourceFactory_);
	materialManager_->SetEnableLighting(true);

	// WVP行列用のリソースを作成（1つのみ）
	wvpResource_ = ResourceFactory::CreateBufferResource(
		sDxCommon_->GetDevice(),
		sizeof(TransformationMatrix)
	);

	// Skeletonをコピー
	if (resource_->GetSkeleton()) {
		skeleton_ = *resource_->GetSkeleton();
		
		const ModelData& modelData = resource_->GetModelData();
		if (!modelData.skinClusterData.empty()) {
			skinCluster_ = SkinClusterGenerator::CreateSkinCluster(
				sDxCommon_->GetDevice(),
				*skeleton_,
				modelData,
				sDxCommon_->GetDescriptorManager()
			);
		}
	}
}

void Model::Initialize(ModelResource* resource, std::unique_ptr<IAnimationController> controller) {
	// 基本の初期化を実行
	Initialize(resource);
	
	// アニメーションコントローラーを設定
	animationController_ = std::move(controller);
}

void Model::UpdateSkinCluster() {
	// SkinClusterとSkeletonが両方存在する場合のみ更新
	if (skinCluster_ && skeleton_) {
		SkinClusterGenerator::Update(*skinCluster_, *skeleton_);
	}
}

void Model::UpdateTransformationMatrix(const WorldTransform& transform, const ICamera* camera) {
	assert(wvpResource_);

	// 行列計算
	Matrix4x4 worldMatrix = transform.GetWorldMatrix();
	Matrix4x4 viewMatrix = camera->GetViewMatrix();
	Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();
	Matrix4x4 worldViewProjectionMatrix = MathCore::Matrix::Multiply(
		worldMatrix, 
		MathCore::Matrix::Multiply(viewMatrix, projectionMatrix)
	);

	// GPUメモリに書き込み
	TransformationMatrix* mappedData = nullptr;
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
	mappedData->world = worldMatrix;
	mappedData->WVP = worldViewProjectionMatrix;
	mappedData->worldInverseTranspose = MathCore::Matrix::Transpose(MathCore::Matrix::Inverse(worldMatrix));
	wvpResource_->Unmap(0, nullptr);
}

void Model::Draw(const WorldTransform& transform, const ICamera* camera,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	
	assert(IsInitialized());
	assert(camera);

	ID3D12GraphicsCommandList* cmdList = sDxCommon_->GetCommandList();
	assert(cmdList);

	// WVP行列を更新（共通処理）
	UpdateTransformationMatrix(transform, camera);

	// スキンクラスターの有無で描画方法を自動判別
	if (HasSkinCluster()) {
		SetupSkinningDrawCommands(cmdList, textureHandle);
	} else {
		SetupNormalDrawCommands(cmdList, textureHandle);
	}

	// 描画実行
	cmdList->DrawIndexedInstanced(resource_->indexCount_, 1, 0, 0, 0);
}

void Model::SetupNormalDrawCommands(ID3D12GraphicsCommandList* cmdList,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	
	// 頂点バッファを設定
	cmdList->IASetVertexBuffers(0, 1, &resource_->vertexBufferView_);
	
	// インデックスバッファを設定
	cmdList->IASetIndexBuffer(&resource_->indexBufferView_);
	
	// マテリアルを設定（Root Parameter 0）
	cmdList->SetGraphicsRootConstantBufferView(
		ModelRendererRootParam::kMaterial, 
		materialManager_->GetGPUVirtualAddress()
	);
	
	// WVP行列を設定（Root Parameter 1）
	cmdList->SetGraphicsRootConstantBufferView(
		ModelRendererRootParam::kWVP,
		wvpResource_->GetGPUVirtualAddress()
	);
	
	// テクスチャを設定（Root Parameter 2）
	cmdList->SetGraphicsRootDescriptorTable(ModelRendererRootParam::kTexture, textureHandle);

	// ライトはBeginPassで自動的にセットされるため、ここでは何もしない
}

void Model::SetupSkinningDrawCommands(ID3D12GraphicsCommandList* cmdList,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	
	assert(skinCluster_.has_value());

	// 頂点バッファを2つ設定（通常の頂点データとInfluenceデータ）
	D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
		resource_->vertexBufferView_,      // Slot 0: VertexData
		skinCluster_->influenceBufferView  // Slot 1: VertexInfluence
	};
	cmdList->IASetVertexBuffers(0, 2, vbvs);
	
	// インデックスバッファを設定
	cmdList->IASetIndexBuffer(&resource_->indexBufferView_);
	
	// WVP行列を設定（Root Parameter 0）
	cmdList->SetGraphicsRootConstantBufferView(
		SkinnedModelRendererRootParam::kWVP, 
		wvpResource_->GetGPUVirtualAddress()
	);
	
	// MatrixPaletteを設定（Root Parameter 1）
	cmdList->SetGraphicsRootDescriptorTable(
		SkinnedModelRendererRootParam::kMatrixPalette,
		skinCluster_->paletteSrvHandle.second
	);
	
	// マテリアルを設定（Root Parameter 2）
	cmdList->SetGraphicsRootConstantBufferView(
		SkinnedModelRendererRootParam::kMaterial,
		materialManager_->GetGPUVirtualAddress()
	);
	
	// テクスチャを設定（Root Parameter 3）
	cmdList->SetGraphicsRootDescriptorTable(SkinnedModelRendererRootParam::kTexture, textureHandle);

	// ライトはBeginPassで自動的にセットされるため、ここでは何もしない
}

void Model::SetUVTransform(const Matrix4x4& uvTransform) {
	if (materialManager_) {
		materialManager_->SetUVTransform(uvTransform);
	}
}

Matrix4x4 Model::GetUVTransform() const {
	if (materialManager_) {
		return materialManager_->GetUVTransform();
	}
	return MathCore::Matrix::Identity();
}

void Model::UpdateAnimation(float deltaTime) {
	if (!animationController_) return;

	// アニメーションの時間を進める
	animationController_->Update(deltaTime);
	
	// SkeletonAnimatorの場合は、スケルトンとスキンクラスターを同期
	if (auto* skeletonAnimator = dynamic_cast<SkeletonAnimator*>(animationController_.get())) {
		skeleton_ = skeletonAnimator->GetSkeleton();
		UpdateSkinCluster();
	}
}

void Model::ResetAnimation() {
	if (animationController_) {
		animationController_->Reset();
	}
}

float Model::GetAnimationTime() const {
	return animationController_ ? animationController_->GetAnimationTime() : 0.0f;
}

bool Model::IsAnimationFinished() const {
	return animationController_ ? animationController_->IsFinished() : true;
}

void Model::SetModelResource(ModelResource* resource) {
   resource_ = resource;
}
