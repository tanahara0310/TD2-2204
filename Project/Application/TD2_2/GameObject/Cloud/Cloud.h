#pragma once
#include "../GameObject.h"
#include <memory>

class Cloud : public GameObject {
public:
	Cloud() = default;
	~Cloud() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

	void Update() override;

	void Draw(const ICamera* camera) override;

	float RangeFloat(float min, float max);

	const char* GetObjectName() const override { return "Cloud"; }

private:

	// 移動速度
	float velocityX_ = 0.0f;

	// 折り返し地点
	const float kEndLineX = 100.0f;
};
