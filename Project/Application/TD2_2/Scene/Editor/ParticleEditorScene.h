#pragma once

#include <memory>
#include "Scene/BaseScene.h"

class EngineSystem;
class Camera;
class ParticleSystem;
class Model;

// パーティクル専用エディターシーン（最小構成）
class ParticleEditorScene : public BaseScene {
public:
    void Initialize(EngineSystem* engine) override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    // エディター用に見やすいカメラに調整
    void SetupReleaseCameraParameters(Camera* camera) override;

private:
    // モデルパーティクル（ボクセル）
    ParticleSystem* modelParticle_ = nullptr;
    std::unique_ptr<Model> voxelModelForParticle_;
};
