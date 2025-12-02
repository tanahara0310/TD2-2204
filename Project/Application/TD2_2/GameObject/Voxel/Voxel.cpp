#include "Voxel.h"
#include <EngineSystem.h>

#ifdef _DEBUG
#include <Windows.h>  // OutputDebugStringW用
#endif

void Voxel::Initialize()
{
	auto engine = GetEngineSystem();

	// 必須コンポーネントの取得
	auto dxCommon = engine->GetComponent<DirectXCommon>();
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	// モデルの作成
	model_ = modelManager->CreateStaticModel("Resources/Models/Voxel/voxel.obj");
	
#ifdef _DEBUG
	if (!model_) {
		OutputDebugStringW(L"[ERROR] Voxel: モデルの読み込みに失敗しました: Resources/Models/Voxel/voxel.obj\n");
	} else {
		OutputDebugStringW(L"[INFO] Voxel: モデルを正常に読み込みました\n");
	}
#endif
	
	// トランスフォームの初期化
	transform_.Initialize(dxCommon->GetDevice());
	
	// テクスチャの読み込み（white1x1）
	texture_ = textureManager.Load("Resources/SampleResources/white1x1.png");
	
#ifdef _DEBUG
	if (texture_.gpuHandle.ptr == 0) {
		OutputDebugStringW(L"[ERROR] Voxel: テクスチャの読み込みに失敗しました: Resources/SampleResources/white1x1.png\n");
	} else {
		OutputDebugStringW(L"[INFO] Voxel: テクスチャを正常に読み込みました\n");
	}
#endif
}

void Voxel::Update()
{
	// トランスフォームの更新
	transform_.TransferMatrix();
}

void Voxel::Draw(const ICamera* camera)
{
	if (!model_ || !camera) {
#ifdef _DEBUG
		if (!model_) {
			OutputDebugStringW(L"[WARNING] Voxel::Draw: モデルがnullptrです\n");
		}
		if (!camera) {
			OutputDebugStringW(L"[WARNING] Voxel::Draw: カメラがnullptrです\n");
		}
#endif
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

bool Voxel::DrawImGui()
{
	return Object3d::DrawImGui();
}
