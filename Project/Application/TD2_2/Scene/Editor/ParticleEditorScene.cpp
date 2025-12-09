#include "ParticleEditorScene.h"

#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "Engine/Particle/ParticleSystem.h"

void ParticleEditorScene::Initialize(EngineSystem* engine) {
    BaseScene::Initialize(engine);

    auto dxCommon = engine_->GetComponent<DirectXCommon>();
    auto resourceFactory = engine_->GetComponent<ResourceFactory>();
    auto modelManager = engine_->GetComponent<ModelManager>();
    if (!dxCommon || !resourceFactory || !modelManager) {
        return;
    }

    // ===== モデルパーティクルシステム（ボクセル）=====
    {
        auto particle = std::make_unique<ParticleSystem>();
        particle->Initialize(dxCommon, resourceFactory);

        // Voxelモデルを読み込む
        voxelModelForParticle_ = modelManager->CreateStaticModel("Resources/Models/Voxel/Voxel.obj");
        
        // ModelResourceを取得してParticleSystemに設定
        auto* voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
        if (voxelModelResource) {
            particle->SetModelResource(voxelModelResource);
        }
        
        // テクスチャを設定
        particle->SetTexture("Resources/SampleResources/white1x1.png");
        
        // モデルパーティクルの基本設定
        particle->SetEmitterPosition({0.0f, 0.0f, 0.0f});
        particle->SetBlendMode(BlendMode::kBlendModeNormal);

        // エディタでは自動削除を無効化（ファイル読み込み時にisLoopingがoffでも残る）
        particle->SetAutoDelete(false);

        particle->GetEmissionModule().Play();
        particle->GetMainModule().Play();

        modelParticle_ = particle.get();
        gameObjects_.push_back(std::move(particle));
    }
}

void ParticleEditorScene::Update() {
    BaseScene::Update();
}

void ParticleEditorScene::Draw() {
    BaseScene::Draw();
}

void ParticleEditorScene::Finalize() {
    BaseScene::Finalize();
}

void ParticleEditorScene::SetupReleaseCameraParameters(Camera* camera) {
    // エディタで見やすい無難な位置
    camera->SetTranslate({ 0.0f, 5.0f, -12.0f });
    camera->SetRotate({ 0.5f, 0.0f, 0.0f });
}
