#include "ParticleSystem.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/Model/ModelResource.h"
#include <iostream>
#ifdef _DEBUG
#include "Engine/Utility/Debug/ImGui/ImguiManager.h"
#endif

using namespace MathCore;

// 初期化関数
void ParticleSystem::Initialize(DirectXCommon* dxCommon, ResourceFactory* resourceFactory)
{
	dxCommon_ = dxCommon;
	resourceFactory_ = resourceFactory;

	// 統一乱数エンジンの初期化
	RandomGenerator::GetInstance().Initialize();

	// モジュールの初期化
	mainModule_ = std::make_unique<MainModule>();
	emissionModule_ = std::make_unique<EmissionModule>();
	shapeModule_ = std::make_unique<ShapeModule>();
	velocityModule_ = std::make_unique<VelocityModule>();
	colorModule_ = std::make_unique<ColorModule>();
	forceModule_ = std::make_unique<ForceModule>();
	sizeModule_ = std::make_unique<SizeModule>();
	rotationModule_ = std::make_unique<RotationModule>();
	noiseModule_ = std::make_unique<NoiseModule>();

	// リソースマネージャーの初期化
	resourceManager_ = std::make_unique<ParticleResourceManager>();
	resourceManager_->Initialize(dxCommon, resourceFactory, kNumMaxInstance);

	// インスタンシングデータのポインタを取得
	instancingData_ = resourceManager_->GetInstancingData();

	// 描画データビルダーの初期化
	renderDataBuilder_ = std::make_unique<ParticleRenderDataBuilder>();

	// パーティクル更新処理の初期化
	particleUpdater_ = std::make_unique<ParticleUpdater>();
	particleUpdater_->Initialize(
		forceModule_.get(),
		colorModule_.get(),
		sizeModule_.get(),
		rotationModule_.get(),
		noiseModule_.get()
	);

	// パーティクル放出処理の初期化
	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize(
		mainModule_.get(),
		emissionModule_.get(),
		shapeModule_.get(),
		velocityModule_.get(),
		rotationModule_.get()
	);

	// エミッタートランスフォームの初期化
	emitterTransform_ = {
		{ 1.0f, 1.0f, 1.0f }, // スケール
		{ 0.0f, 0.0f, 0.0f }, // 回転
		{ 0.0f, 0.0f, 0.0f }  // 平行移動
	};

	// デフォルトテクスチャを設定（存在するパスに変更）
	SetTexture("Resources/SampleResources/circle.png");
}

// 更新処理関数（他のオブジェクトと統一）
void ParticleSystem::Update()
{
	const float kDeltaTime = 1.0f / 60.0f;

	// 統計情報の更新
	statistics_.systemRuntime += kDeltaTime;
	deltaTimeAccumulator_ += kDeltaTime;

	// MainModuleの時間更新
	mainModule_->UpdateTime(kDeltaTime);

	// MainModuleの時間とループ設定を取得
	float elapsedTime = mainModule_->GetElapsedTime();
	float duration = mainModule_->GetMainData().duration;
	bool looping = mainModule_->GetMainData().looping;

	// EmissionModuleの時間更新（バースト用）
	emissionModule_->UpdateTime(kDeltaTime);

	// MainModuleがループでリセットされた場合、EmissionModuleもリセット
	if (looping && elapsedTime < lastElapsedTime_) {
		// ループがリセットされた
		emissionModule_->Play();  // EmissionModuleを再開
	}
	lastElapsedTime_ = elapsedTime;

	// ループしない場合、duration超過でEmissionを停止
	bool shouldEmit = emissionModule_->IsPlaying();
	if (!looping && elapsedTime >= duration) {
		// バーストがまだ発生していない場合は、バーストを発生させてから停止
		const auto& emissionData = emissionModule_->GetEmissionData();
		if (emissionData.burstCount > 0 && emissionData.burstTime >= duration) {
			// バーストタイミングがduration以降の場合、duration到達時に強制発生
			uint32_t burstCount = emissionModule_->CalculateEmissionCount(kDeltaTime);
			if (burstCount > 0) {
				uint32_t emittedCount = particleEmitter_->EmitParticles(
					burstCount,
					emitterTransform_,
					GetMaxParticleCount(),
					particles_
				);
				statistics_.totalParticlesCreated += emittedCount;
			}
		}
		shouldEmit = false;
		emissionModule_->Stop();  // Emissionを明示的に停止
	} else if (looping && !shouldEmit) {
		// ループ中でEmissionが停止している場合は再開
		emissionModule_->Play();
	}

	// 放出処理
	uint32_t emissionCount = 0;
	if (shouldEmit) {
		emissionCount = emissionModule_->CalculateEmissionCount(kDeltaTime);
		if (emissionCount > 0) {
			uint32_t emittedCount = particleEmitter_->EmitParticles(
				emissionCount,
				emitterTransform_,
				GetMaxParticleCount(),
				particles_
			);
			statistics_.totalParticlesCreated += emittedCount;
		}
	}

	// MainModuleからgravityModifierを取得
	float gravityModifier = mainModule_->GetMainData().gravityModifier;

	// パーティクルの更新（ParticleUpdaterに委譲）
	uint32_t destroyedCountFromUpdate = particleUpdater_->UpdateParticles(particles_, kDeltaTime, gravityModifier);

	// パーティクルの更新後の統計情報を更新
	uint32_t currentParticleCount = GetParticleCount();
	if (currentParticleCount > statistics_.peakParticleCount) {
		statistics_.peakParticleCount = currentParticleCount;
	}

	// 破棄されたパーティクル数を統計に反映
	statistics_.totalParticlesDestroyed += destroyedCountFromUpdate;

	// 平均ライフタイムの計算（1秒ごとに更新）
	if (deltaTimeAccumulator_ >= 1.0f) {
		if (statistics_.totalParticlesDestroyed > 0) {
			// MainModuleから平均寿命を取得
			statistics_.averageLifetime = mainModule_->GetMainData().startLifetime;
		}
		deltaTimeAccumulator_ = 0.0f;
	}
}

// 描画関数（Object3dと同じインターフェース）
void ParticleSystem::Draw(const ICamera* camera)
{
	if (!camera) return;

	// 描画データを準備（ビルボード計算などを担当）
	instanceCount_ = renderDataBuilder_->BuildRenderData(
		particles_,
		camera,
		billboardType_,
		renderMode_,
		instancingData_,
		kNumMaxInstance
	);
}

void ParticleSystem::Play()
{
	mainModule_->Play();
	emissionModule_->Play();
}

void ParticleSystem::Stop()
{
	mainModule_->Stop();
	emissionModule_->Stop();
}

bool ParticleSystem::IsPlaying() const
{
	return mainModule_->IsPlaying() && emissionModule_->IsPlaying();
}

void ParticleSystem::Clear()
{
	particles_.clear();
	instanceCount_ = 0;
}

bool ParticleSystem::IsFinished() const
{
	// 再生が停止していて、かつパーティクルが0個の場合は終了
	return !emissionModule_->IsPlaying() && particles_.empty();
}

bool ParticleSystem::CanBeDeleted() const
{
	// 非アクティブなオブジェクトは削除しない（手動管理）
	if (!IsActive()) {
		return false;
	}
	
	// ワンショット（ループなし）の場合、終了したら削除可能
	const auto& mainData = mainModule_->GetMainData();
	if (!mainData.looping) {
		return IsFinished();
	}
	// ループする場合は手動で削除する必要がある
	return false;
}

void ParticleSystem::SetTexture(const std::string& texturePath)
{
	texture_ = TextureManager::GetInstance().Load(texturePath);
}

void ParticleSystem::SetModelResource(ModelResource* modelResource)
{
	modelResource_ = modelResource;
	if (modelResource_) {
		renderMode_ = ParticleRenderMode::Model;
		// モデルパーティクルではビルボードを無効化
		billboardType_ = BillboardType::None;
	} else {
		renderMode_ = ParticleRenderMode::Billboard;
	}
}

bool ParticleSystem::DrawImGui()
{
#ifdef _DEBUG
	return ParticleSystemDebugUI::ShowImGui(this);
#else
	return false;
#endif
}
