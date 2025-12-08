#pragma once

#include <memory>
#include "Scene/BaseScene.h"

class EngineSystem;
class Camera;
class ParticleSystem;

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
    ParticleSystem* particleSystem_ = nullptr; // 所有は BaseScene::gameObjects_
};
