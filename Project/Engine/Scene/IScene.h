#pragma once
#include <string>

class EngineSystem; // 前方宣言
class SceneManager; // 前方宣言

/// @brief シーンインターフェース
class IScene {
public:
    virtual ~IScene() = default;

    virtual void Initialize(EngineSystem* engine) = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Finalize() = 0;

    /// @brief SceneManager への参照を設定（自動呼び出し）
    virtual void SetSceneManager(SceneManager* sceneManager) {
        sceneManager_ = sceneManager;
    }

protected:
    SceneManager* sceneManager_ = nullptr;
};
