#pragma once
#include "../GameObject.h"
#include <memory>

enum class CloudDirection { RIGHT, LEFT };

class Cloud : public GameObject {
public:
	Cloud() = default;
	~Cloud() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, CloudDirection direction);

	void Update() override;

	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "Cloud"; }

	CloudDirection GetDirection() { return direction_; }

	void SetDirection(CloudDirection direction) { direction_ = direction; }

private:
	// 進行方向
	CloudDirection direction_ = CloudDirection::LEFT;

	// 移動速度
	float velocityX_ = 1.0f;
};
