#include <EngineSystem.h>


#include "TestScene.h"
#include "WinApp/WinApp.h"
#include "Scene/SceneManager.h"
#include "Engine/Graphics/Model/Skeleton/SkeletonDebugRenderer.h"

#include <iostream>

using namespace MathCore;
using namespace CollisionUtils;

// アプリケーションの初期化
void TestScene::Initialize(EngineSystem* engine)
{

	BaseScene::Initialize(engine);

	///========================================================
	// モデルの読み込みと初期化
	///========================================================

	// コンポーネントを直接取得
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	auto modelManager = engine_->GetComponent<ModelManager>();

	if (!dxCommon || !modelManager) {
		return; // 必須コンポーネントがない場合は終了
	}


	// ===== 3Dゲームオブジェクトの生成と初期化=====
	// Sphereオブジェクト
	auto sphere = std::make_unique<SphereObject>();
	sphere->Initialize();
	sphere->SetActive(false);  // Skeletonテスト中は非表示
	gameObjects_.push_back(std::move(sphere));

	// Fenceオブジェクト
	auto fence = std::make_unique<FenceObject>();
	fence->Initialize();
	fence->SetActive(false);  // Skeletonテスト中は非表示
	gameObjects_.push_back(std::move(fence));

	// Terrainオブジェクト
	auto terrain = std::make_unique<TerrainObject>();
	terrain->Initialize();
	terrain->SetActive(false);  // Skeletonテスト中は非表示
	gameObjects_.push_back(std::move(terrain));

	// AnimatedCubeオブジェクト
	auto animatedCube = std::make_unique<AnimatedCubeObject>();
	animatedCube->Initialize();
	animatedCube->SetActive(false);  // Skeletonテスト中は非表示
	gameObjects_.push_back(std::move(animatedCube));

	// SkeletonModelオブジェクト
	auto skeletonModel = std::make_unique<SkeletonModelObject>();
	skeletonModel->Initialize();
	skeletonModel->SetActive(true);
	gameObjects_.push_back(std::move(skeletonModel));

	// WalkModelオブジェクト
	auto walkModel = std::make_unique<WalkModelObject>();
	walkModel->Initialize();
	walkModel->SetActive(true);
	gameObjects_.push_back(std::move(walkModel));

	// SneakWalkModelオブジェクト
	auto sneakWalkModel = std::make_unique<SneakWalkModelObject>();
	sneakWalkModel->Initialize();
	sneakWalkModel->SetActive(true);
	gameObjects_.push_back(std::move(sneakWalkModel));

	// SkyBoxの初期化（gameObjects_に追加）
	auto skyBox = std::make_unique<SkyBoxObject>();
	skyBox->Initialize();
	gameObjects_.push_back(std::move(skyBox));

	// スプライトオブジェクトの初期化（複数作成）
	auto sprite1 = std::make_unique<SpriteObject>();
	sprite1->Initialize("Resources/SampleResources/uvChecker.png");
	sprite1->GetTransform().translate = { 100.0f, 100.0f, 0.0f };
	sprite1->GetTransform().scale = { 0.5f, 0.5f, 1.0f };
	gameObjects_.push_back(std::move(sprite1));

	// スプライト2: circle
	auto sprite2 = std::make_unique<SpriteObject>();
	sprite2->Initialize("Resources/SampleResources/circle.png");
	sprite2->GetTransform().translate = { 400.0f, 200.0f, 0.0f };
	sprite2->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
	gameObjects_.push_back(std::move(sprite2));

	// スプライト3: 別のuvChecker（異なる位置）
	auto sprite3 = std::make_unique<SpriteObject>();
	sprite3->Initialize("Resources/SampleResources/uvChecker.png");
	sprite3->GetTransform().translate = { 700.0f, 400.0f, 0.0f };
	sprite3->GetTransform().scale = { 0.8f, 0.8f, 1.0f };
	gameObjects_.push_back(std::move(sprite3));

	// ===== パーティクルシステムの初期化 =====
	auto particleSystem = std::make_unique<ParticleSystem>();
	particleSystem->Initialize(dxCommon, engine_->GetComponent<ResourceFactory>());

	// パーティクルシステムの設定
	particleSystem ->SetTexture("Resources/SampleResources/circle.png");
	particleSystem->SetEmitterPosition({ 0.0f, 2.0f, 0.0f });
	particleSystem->SetBlendMode(BlendMode::kBlendModeAdd);  // 加算合成
	particleSystem->SetBillboardType(BillboardType::ViewFacing);
	particleSystem_= particleSystem.get();

	gameObjects_.push_back(std::move(particleSystem));

	// エミッションモジュールの設定
	{
		auto& emissionModule = particleSystem_->GetEmissionModule();
		auto emissionData = emissionModule.GetEmissionData();
		emissionData.rateOverTime = 20;  // 1秒に20個のパーティクルを放出
		emissionData.shapeType = EmissionModule::ShapeType::Sphere;
		emissionData.radius = 0.5f;
		emissionData.emitFromSurface = false;
		emissionModule.SetEmissionData(emissionData);
	}

	// 速度モジュールの設定
	{
		auto& velocityModule = particleSystem_->GetVelocityModule();
		auto velocityData = velocityModule.GetVelocityData();
		velocityData.startSpeed = { 0.0f, 1.0f, 0.0f };
		velocityData.randomSpeedRange = { 1.0f, 1.0f, 1.0f };
		velocityData.useRandomDirection = true;
		velocityModule.SetVelocityData(velocityData);
	}

	// 色モジュールの設定
	{
		auto& colorModule = particleSystem_->GetColorModule();
		auto colorData = colorModule.GetColorData();
		colorData.useGradient = true;
		colorData.startColor = { 1.0f, 0.8f, 0.2f, 1.0f };  // 黄色
		colorData.endColor = { 1.0f, 0.2f, 0.0f, 0.0f };    // 赤でフェードアウト
		colorModule.SetColorData(colorData);
	}

	// ライフタイムモジュールの設定
	{
		auto& lifetimeModule = particleSystem_->GetLifetimeModule();
		auto lifetimeData = lifetimeModule.GetLifetimeData();
		lifetimeData.startLifetime = 2.0f;
		lifetimeData.lifetimeRandomness = 0.25f;
		lifetimeModule.SetLifetimeData(lifetimeData);
	}

	// サイズモジュールの設定
	{
		auto& sizeModule = particleSystem_->GetSizeModule();
		auto sizeData = sizeModule.GetSizeData();
		sizeData.startSize = 0.3f;
		sizeData.endSize = 0.05f;
		sizeData.sizeOverLifetime = true;
		sizeModule.SetSizeData(sizeData);
	}

	// パーティクルシステムを再生開始
	particleSystem_->Play();

	// テクスチャの読み込み
	auto& textureManager = TextureManager::GetInstance();
	textureChecker_ = textureManager.Load("Resources/SampleResources/uvChecker.png");
	textureCircle_ = textureManager.Load("Resources/SampleResources/circle.png");

	// サウンドリソースを取得
	auto soundManager = engine_->GetComponent<SoundManager>();
	if (soundManager) {
		mp3Resource_ = soundManager->CreateSoundResource("Resources/Audio/BGM/test.mp3");
	}

}

void TestScene::Update()
{

	BaseScene::Update();

	// KeyboardInput を直接取得
	auto keyboard = engine_->GetComponent<KeyboardInput>();
	if (!keyboard) {
		return;
	}

	// Tabキーでテストシーンをリスタート
	if (keyboard->IsKeyTriggered(DIK_TAB)) {
		if (sceneManager_) {
			sceneManager_->ChangeScene("TestScene");
		}
		return;
	}

	// ===== スペースキーでMP3再生　====
	if (keyboard->IsKeyTriggered(DIK_SPACE)) {
		if (mp3Resource_ && mp3Resource_->IsValid()) {
			bool isPlaying = mp3Resource_->IsPlaying();
			if (isPlaying) {
				mp3Resource_->Stop();
			} else {
				mp3Resource_->Play(false);
			}
		}
	}
}

void TestScene::Draw()
{
	BaseScene::Draw();
}


void TestScene::Finalize()
{
}