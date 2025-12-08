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
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "Engine/Particle/ParticleSystem.h"

void ParticleEditorScene::Initialize(EngineSystem* engine) {
    BaseScene::Initialize(engine);

    // 必要最小限: パーティクルシステムのみ生成
    auto dxCommon = engine_->GetComponent<DirectXCommon>();
    auto resourceFactory = engine_->GetComponent<ResourceFactory>();
    if (!dxCommon || !resourceFactory) {
        return;
    }

    auto particle = std::make_unique<ParticleSystem>();
    particle->Initialize(dxCommon, resourceFactory);

    // 最小設定（動作確認用）。詳細な調整はデバッグUIで行う
    particle->SetEmitterPosition({0.0f, 0.0f, 0.0f});
    particle->SetBlendMode(BlendMode::kBlendModeAdd);

    // そのままUI操作で調整できるように再生だけ開始
    particle->GetEmissionModule().Play();
    particle->GetMainModule().Play();

    particleSystem_ = particle.get();
    gameObjects_.push_back(std::move(particle));
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
